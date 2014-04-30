//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <iomanip>
#include <istream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

//#include "boost_cfg.hpp"

//#define BOOST_SPIRIT_X3_DEBUG

#include <boost/spirit/home/x3.hpp>

//#include "GlobalContext.hpp"

#include "parser_x3/SpiritParser.hpp"
#include "parser_x3/ast.hpp"
#include "parser_x3/printer.hpp"
#include "parser_x3/utils.hpp"
#include "parser_x3/iterator.hpp"
#include "parser_x3/type_grammar.hpp"
#include "parser_x3/value_grammar.hpp"
#include "parser_x3/instruction_grammar.hpp"

namespace x3 = boost::spirit::x3;

using namespace eddic;

template<typename ForwardIteratorT>
class extended_iterator : public boost::spirit::classic::position_iterator2<ForwardIteratorT> {
    std::size_t current_file;

public:
    extended_iterator() : boost::spirit::classic::position_iterator2<ForwardIteratorT>() {
        //Nothing
    }

    template <typename FileNameT>
    extended_iterator(
        const ForwardIteratorT& begin, const ForwardIteratorT& end,
        FileNameT file, std::size_t current_file)
            : boost::spirit::classic::position_iterator2<ForwardIteratorT>(begin, end, file), current_file(current_file)
    {}

    std::size_t get_current_file() const {
        return current_file;
    }

    extended_iterator(const extended_iterator& iter) = default;
    extended_iterator& operator=(const extended_iterator& iter) = default;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"

namespace x3_grammar {
    typedef x3::identity<struct source_file> source_file_id;
    typedef x3::identity<struct function_parameter> function_parameter_id;
    typedef x3::identity<struct template_function_declaration> template_function_declaration_id;
    typedef x3::identity<struct global_variable_declaration> global_variable_declaration_id;
    typedef x3::identity<struct global_array_declaration> global_array_declaration_id;
    typedef x3::identity<struct import> import_id;
    typedef x3::identity<struct standard_import> standard_import_id;
    typedef x3::identity<struct member_declaration> member_declaration_id;
    typedef x3::identity<struct template_struct> template_struct_id;

    x3::rule<source_file_id, x3_ast::source_file> const source_file("source_file");
    x3::rule<function_parameter_id, x3_ast::function_parameter> const function_parameter("function_parameter");
    x3::rule<template_function_declaration_id, x3_ast::template_function_declaration> const template_function_declaration("template_function_declaration");
    x3::rule<global_variable_declaration_id, x3_ast::global_variable_declaration> const global_variable_declaration("global_variable_declaration");
    x3::rule<global_array_declaration_id, x3_ast::global_array_declaration> const global_array_declaration("global_array_declaration");
    x3::rule<standard_import_id, x3_ast::standard_import> const standard_import("standard_import");
    x3::rule<import_id, x3_ast::import> const import("import");
    x3::rule<member_declaration_id, x3_ast::member_declaration> const member_declaration("member_declaration");
    x3::rule<template_struct_id, x3_ast::template_struct> const template_struct("template_struct");

    ANNOTATE(import_id);
    ANNOTATE(standard_import_id);
    ANNOTATE(template_function_declaration_id);
    ANNOTATE(global_variable_declaration_id);
    ANNOTATE(global_array_declaration_id);
    ANNOTATE(member_declaration_id);
    ANNOTATE(template_struct_id);

    #include "parser_x3/skipper_inc.hpp"
    #include "parser_x3/identifier_inc.hpp"
    
    auto const source_file_def = 
         *(
                standard_import
            |   import
            |   template_struct
            |   template_function_declaration
            |   (global_array_declaration > ';')
            |   (global_variable_declaration > ';')
         );

    auto const standard_import_def = 
            x3::lit("import")
        >>  '<' 
        >   *x3::alpha
        >   '>';
    
    auto const import_def = 
            x3::lit("import")
        >>  '"' 
        >   *x3::alpha
        >   '"';
    
    auto const template_function_declaration_def = 
            -(
                    x3::lit("template")
                >>  '<'
                >>  (x3::lit("type") >> identifier) % ','
                >>  '>'
            )
        >>  type_grammar 
        >>  identifier
        >>  '(' 
        >>  -(function_parameter % ',')
        >   ')'
        >   '{' 
        >   *instruction_grammar
        >   '}';

    auto const global_variable_declaration_def =
            type_grammar
        >>  identifier
        >>  -('=' >> value_grammar);
    
    auto const global_array_declaration_def =
            type_grammar
        >>  identifier
        >>  '['
        >>  value_grammar
        >>  ']';

    auto const function_parameter_def =
            type_grammar
        >>  identifier;

    auto const member_declaration_def =
            type_grammar
        >>  identifier
        >>  ';';

    auto template_struct_def =
            -(  
                    x3::lit("template")
                >>  '<'
                >>  (x3::lit("type") >> identifier) % ','
                >>  '>'
            )
        >>  x3::lit("struct")
        >>  identifier
        >>  -(
                    "extends" 
                >>  type_grammar
             )
        >>  '{'
        >>  *(
                    member_declaration
                //|   (array_declaration >> ';')
                |   template_function_declaration
             )
        >>  '}';

    /* Grammar */
    
    auto const parser = x3::grammar(
        "eddi", 
        source_file = source_file_def,
        function_parameter = function_parameter_def, 
        template_function_declaration = template_function_declaration_def, 
        global_variable_declaration = global_variable_declaration_def,
        global_array_declaration = global_array_declaration_def,
        standard_import = standard_import_def,
        import = import_def,
        member_declaration = member_declaration_def,
        //array_declaration = array_declaration_def,
        template_struct = template_struct_def
        );

} // end of grammar namespace

#pragma clang diagnostic pop

bool parser_x3::SpiritParser::parse(const std::string& file/*, ast::SourceFile& , std::shared_ptr<GlobalContext> context*/){
    //timing_timer timer(context->timing(), "parsing_x3");

    std::ifstream in(file.c_str(), std::ios::binary);
    in.unsetf(std::ios::skipws);

    //Collect the size of the file
    in.seekg(0, std::istream::end);
    std::size_t size(static_cast<size_t>(in.tellg()));
    in.seekg(0, std::istream::beg);

    //int current_file = context->new_file(file);

    //std::string& file_contents = context->get_file_content(current_file);
    std::string file_contents;
    file_contents.resize(size);
    in.read(&file_contents[0], size);

    auto& parser = x3_grammar::parser;
    auto& skipper = x3_grammar::skipper;
    //x3::ascii::space_type skipper

    x3_ast::source_file result;
    //boost::spirit::x3::ascii::space_type space;

    pos_iterator_type it(file_contents.begin(), file_contents.end(), file);
    pos_iterator_type end;

    try {
        bool r = x3::phrase_parse(it, end, parser, skipper, result);

        if(r && it == end){
            x3_ast::printer printer;
            printer(result);

            return true;
        } else {
            auto& pos = it.get_position();
            std::cout <<
                "parse error at file " << pos.file <<
                " line " << pos.line << " column " << pos.column << std::endl <<
                "'" << it.get_currentline() << "'" << std::endl <<
                std::setw(pos.column) << " " << "^- here" << std::endl;

            return false;
        }
    } catch(const boost::spirit::x3::expectation_failure<pos_iterator_type>& e){
        auto& pos = e.first.get_position();
        std::cout <<
                "parse error at file " << pos.file << " line " << pos.line << " column " << pos.column << std::endl
            <<  "expected: " << e.what_ << std::endl
            <<  "'" << e.first.get_currentline() << "'" << std::endl
            <<  std::setw(pos.column) << " " << "^- here" << std::endl;

        //TODO The position seems really off

        return false;
    }
}
