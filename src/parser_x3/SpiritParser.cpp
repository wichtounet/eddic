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
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

//#include "GlobalContext.hpp"

#include "parser_x3/SpiritParser.hpp"
#include "parser_x3/ast.hpp"
#include "parser_x3/printer.hpp"

namespace x3 = boost::spirit::x3;

typedef std::string::iterator base_iterator_type;
typedef boost::spirit::classic::position_iterator2<base_iterator_type> pos_iterator_type;
//typedef extended_iterator<base_iterator_type> pos_iterator_type;

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

#define ANNOTATE(Type)\
template <typename iterator_type, typename Attr, typename Context>\
inline void on_success(Type, const iterator_type& first, const iterator_type&, Attr& attr, Context const&){\
    auto& pos = first.get_position();\
    attr.pos.file = pos.file;\
    attr.pos.line = pos.line;\
    attr.pos.column = pos.column;\
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"

namespace x3_grammar {
    typedef x3::identity<struct source_file> source_file_id;

    typedef x3::identity<struct type> type_id;
    typedef x3::identity<struct simple_type> simple_type_id;
    typedef x3::identity<struct array_type> array_type_id;
    typedef x3::identity<struct pointer_type> pointer_type_id;
    typedef x3::identity<struct template_type> template_type_id;
    
    typedef x3::identity<struct integer_literal> integer_literal_id;
    typedef x3::identity<struct integer_suffix_literal> integer_suffix_literal_id;
    typedef x3::identity<struct float_literal> float_literal_id;
    typedef x3::identity<struct string_literal> string_literal_id;
    typedef x3::identity<struct char_literal> char_literal_id;
    typedef x3::identity<struct variable_value> variable_value_id;
    typedef x3::identity<struct value> value_id;
    
    typedef x3::identity<struct instruction> instruction_id;
    typedef x3::identity<struct foreach> foreach_id;
    typedef x3::identity<struct foreach_in> foreach_in_id;
    typedef x3::identity<struct while_> while_id;
    typedef x3::identity<struct do_while> do_while_id;
    typedef x3::identity<struct variable_declaration> variable_declaration_id;
    typedef x3::identity<struct struct_declaration> struct_declaration_id;
    typedef x3::identity<struct array_declaration> array_declaration_id;
    typedef x3::identity<struct return_> return_id;
    typedef x3::identity<struct delete_> delete_id;
    typedef x3::identity<struct if_> if_id;
    typedef x3::identity<struct else_if> else_if_id;
    typedef x3::identity<struct else_> else_id;

    typedef x3::identity<struct function_parameter> function_parameter_id;
    typedef x3::identity<struct template_function_declaration> template_function_declaration_id;
    typedef x3::identity<struct global_variable_declaration> global_variable_declaration_id;
    typedef x3::identity<struct global_array_declaration> global_array_declaration_id;
    typedef x3::identity<struct import> import_id;
    typedef x3::identity<struct standard_import> standard_import_id;
    typedef x3::identity<struct member_declaration> member_declaration_id;
    typedef x3::identity<struct template_struct> template_struct_id;

    x3::rule<source_file_id, x3_ast::source_file> const source_file("source_file");

    x3::rule<type_id, x3_ast::type> const type("type");
    x3::rule<simple_type_id, x3_ast::simple_type> const simple_type("simple_type");
    x3::rule<array_type_id, x3_ast::array_type> const array_type("array_type");
    x3::rule<pointer_type_id, x3_ast::pointer_type> const pointer_type("pointer_type");
    x3::rule<template_type_id, x3_ast::template_type> const template_type("template_type");
    
    x3::rule<integer_literal_id, x3_ast::integer_literal> const integer_literal("integer_literal");
    x3::rule<integer_suffix_literal_id, x3_ast::integer_suffix_literal> const integer_suffix_literal("integer_suffix_literal");
    x3::rule<float_literal_id, x3_ast::float_literal> const float_literal("float_literal");
    x3::rule<string_literal_id, x3_ast::string_literal> const string_literal("string_literal");
    x3::rule<char_literal_id, x3_ast::char_literal> const char_literal("char_literal");
    x3::rule<variable_value_id, x3_ast::variable_value> const variable_value("variable_value");
    x3::rule<value_id, x3_ast::value> const value("value");
    
    x3::rule<instruction_id, x3_ast::instruction> const instruction("instruction");
    x3::rule<foreach_id, x3_ast::foreach> const foreach("foreach");
    x3::rule<foreach_in_id, x3_ast::foreach_in> const foreach_in("foreach_in");
    x3::rule<while_id, x3_ast::while_> const while_("while");
    x3::rule<do_while_id, x3_ast::do_while> const do_while("do_while");
    x3::rule<variable_declaration_id, x3_ast::variable_declaration> const variable_declaration("variable_declaration");
    x3::rule<struct_declaration_id, x3_ast::struct_declaration> const struct_declaration("struct_declaration");
    x3::rule<array_declaration_id, x3_ast::array_declaration> const array_declaration("array_declaration");
    x3::rule<return_id, x3_ast::return_> const return_("return");
    x3::rule<delete_id, x3_ast::delete_> const delete_("delete");
    x3::rule<if_id, x3_ast::if_> const if_("if");
    x3::rule<else_if_id, x3_ast::else_if> const else_if("else_if");
    x3::rule<else_id, x3_ast::else_> const else_("else");

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
    ANNOTATE(foreach_id);
    ANNOTATE(foreach_in_id);
    ANNOTATE(while_id);
    ANNOTATE(do_while_id);
    ANNOTATE(variable_value_id);
    ANNOTATE(variable_declaration_id);
    ANNOTATE(struct_declaration_id);
    ANNOTATE(array_declaration_id);
    ANNOTATE(return_id);
    ANNOTATE(delete_id);
    ANNOTATE(global_variable_declaration_id);
    ANNOTATE(global_array_declaration_id);
    ANNOTATE(member_declaration_id);
    ANNOTATE(template_struct_id);

    /* Utilities */

    auto const skipper = 
            x3::ascii::space
        |   ("/*" >> *(x3::char_ - "*/") >> "*/")
        |   ("//" >> *(x3::char_ - (x3::eol | x3::eoi)) >> (x3::eol | x3::eoi));
   
    auto const const_ = 
            (x3::lit("const") > x3::attr(true))
        |   x3::attr(false);

    x3::real_parser<double, x3::strict_real_policies<double>> strict_double;

    auto const identifier = 
                x3::lexeme[(x3::char_('_') >> *(x3::alnum | x3::char_('_')))]
            |   x3::lexeme[(x3::alpha >> *(x3::alnum | x3::char_('_')))]
            ;

    /* Types */ 

    auto const type_def =
            array_type
        |   pointer_type
        |   template_type
        |   simple_type;

    auto const simple_type_def = 
            const_
        >>  identifier;

    auto const template_type_def =
            identifier 
        >>  '<'
        >>  type % ','
        >>  '>';
    
    auto const array_type_def =
            (
                    template_type
                |   simple_type
            )
        >>  '['
        >>  ']';

    auto const pointer_type_def =
           (
                    template_type
                |   simple_type
            )
        >>  '*';

    using type_parser_type = x3::any_parser<pos_iterator_type, x3_ast::type>;

    type_parser_type type_grammar_create(){
        return x3::skip(skipper)[x3::grammar(
            "eddi::type",
            type = type_def,
            simple_type = simple_type_def,
            template_type = template_type_def,
            array_type = array_type_def,
            pointer_type = pointer_type_def
            )];
    }

    auto const type_grammar = type_grammar_create();
    
    /* Values */ 

    auto const integer_literal_def =
        x3::int_;
    
    auto const integer_suffix_literal_def =
        x3::lexeme[
                x3::int_ 
            >>  +x3::alpha
        ];

    auto const float_literal_def =
        strict_double;
    
    auto const char_literal_def =
            x3::lit('\'')
        >>  x3::char_
        >>  x3::lit('\'');
    
    auto const string_literal_def =
            x3::lit('"') 
        >>  x3::no_skip[*(x3::char_ - '"')] 
        >>  x3::lit('"');

    auto const variable_value_def =
        identifier;

    auto const value_def =
            variable_value
        |   integer_suffix_literal
        |   float_literal
        |   integer_literal
        |   string_literal
        |   char_literal;

    using value_parser_type = x3::any_parser<pos_iterator_type, x3_ast::value>;

    value_parser_type value_grammar_create(){
        return x3::skip(skipper)[x3::grammar(
            "eddi::value",
            value = value_def,
            integer_literal = integer_literal_def,
            integer_suffix_literal = integer_suffix_literal_def,
            float_literal = float_literal_def,
            char_literal = char_literal_def,
            string_literal = string_literal_def,
            variable_value = variable_value_def
            )];
    }

    auto const value_grammar = value_grammar_create();
    
    /* Instructions */

    auto const instruction_def =
            foreach
        |   foreach_in
        //|   if_
        |   while_
        |   do_while
        |   (return_ > ';')
        |   (delete_ > ';')
        |   (struct_declaration > ';')
        |   (array_declaration > ';')
        |   (variable_declaration > ';');

    auto const foreach_def =
            x3::lit("foreach")
        >>  '('
        >>  type_grammar
        >>  identifier
        >>  "from"
        >>  x3::int_
        >>  "to"
        >>  x3::int_
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}';
    
    auto const foreach_in_def =
            x3::lit("foreach")
        >>  '('
        >>  type_grammar
        >>  identifier
        >>  "in"
        >>  identifier
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}';
    
    auto const while_def =
            x3::lit("while")
        >>  '('
        >>  value_grammar
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}';
    
    auto const do_while_def =
            x3::lit("do")
        >>  '{'
        >>  *instruction
        >>  '}'
        >>  "while"
        >>  '('
        >>  value_grammar
        >>  ')'
        >>  ';';
    
    auto const variable_declaration_def =
            type_grammar
        >>  identifier
        >>  -('=' >> value_grammar);
    
    auto const struct_declaration_def =
            type_grammar
        >>  identifier
        >>  '('
        >>  -(value_grammar % ',')
        >>  ')';
    
    auto const array_declaration_def =
            type_grammar
        >>  identifier
        >>  '['
        >>  value_grammar
        >>  ']';

    auto const return_def =
            x3::lit("return")
        >>  x3::attr(1)
        >>  value_grammar;

    auto const delete_def =
            x3::lit("delete")
        >>  x3::attr(1)
        >>  value_grammar;

    auto const if_def =
            x3::lit("if")
        >>  '('
        >>  value_grammar
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}'
        >>  *else_if
        >>  -else_;

    auto const else_if_def =
            x3::lit("else")
        >>  x3::lit("if")
        >>  '('
        >>  value_grammar
        >>  ')'
        >>  '{'
        >>  *instruction
        >>  '}';
    
    auto const else_def =
            x3::lit("else")
        >>  x3::attr(1)
        >>  '{'
        >>  *instruction
        >>  '}';
    
    using instruction_parser_type = x3::any_parser<pos_iterator_type, x3_ast::instruction>;

    instruction_parser_type instruction_grammar_create(){
        return x3::skip(skipper)[x3::grammar(
            "eddi::instruction",
            instruction = instruction_def,
            foreach = foreach_def,
            foreach_in = foreach_in_def,
            while_ = while_def,
            do_while = do_while_def,
            variable_declaration = variable_declaration_def,
            struct_declaration = struct_declaration_def,
            array_declaration = array_declaration_def,
            return_ = return_def,
            delete_ = delete_def,
            if_ = if_def,
            else_if = else_if_def,
            else_ = else_def
            )];
    }

    auto const instruction_grammar = instruction_grammar_create();

    /* Base */ 
    
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
                |   (array_declaration >> ';')
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
        array_declaration = array_declaration_def,
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
