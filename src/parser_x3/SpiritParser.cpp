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

#include <boost/mpl/vector.hpp>
#include <boost/mpl/count.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

//#include "GlobalContext.hpp"

#include "parser_x3/SpiritParser.hpp"

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

namespace x3_ast {

struct position {
    std::size_t file = 0;               /*!< The source file */
    std::size_t line = 0;               /*!< The source line number */
    std::size_t column = 0;             /*!< The source column number */
};

//*****************************************

struct simple_type;
struct array_type;
struct pointer_type;
struct template_type;

typedef x3::variant<
        simple_type, 
        x3::forward_ast<array_type>, 
        x3::forward_ast<template_type>, 
        x3::forward_ast<pointer_type>
    > type;

struct simple_type {
    bool const_;
    std::string base_type;
};

struct array_type {
    type base_type;
};

struct pointer_type {
    type base_type;
};

struct template_type {
    std::string base_type;
    std::vector<type> template_types;
};

//*****************************************

struct foreach;
struct foreach_in;

typedef x3::variant<
        x3::forward_ast<foreach>,
        x3::forward_ast<foreach_in>
    > instruction;

struct foreach_in {
    position pos;
    type variable_type;
    std::string variable_name;
    std::string array_name;
    std::vector<instruction> instructions;
};

struct foreach {
    position pos;
    type variable_type;
    std::string variable_name;
    int from;
    int to;
    std::vector<instruction> instructions;
};

//*****************************************

struct function_parameter {
    type  parameter_type;
    std::string parameter_name;
};

struct function_declaration {
    position pos;
    type return_type;
    std::string name;
    std::vector<function_parameter> parameters;
    std::vector<instruction> instructions;
};

struct standard_import {
    position pos;
    std::string file;
};

struct import {
    position pos;
    std::string file;
};

typedef x3::variant<
        function_declaration,
        standard_import,
        import
    > block;

struct source_file {
    std::vector<block> blocks;
};

struct printer: public boost::static_visitor<>  {
    std::size_t i = 0;

    std::string indent(){
        std::string v(i, ' ');
        return v;
    }

    void operator()(const source_file& source_file){
        std::cout << indent() << "source_file" << std::endl;

        i += 2;
        for(auto& block : source_file.blocks){
            boost::apply_visitor(*this, block);
        }
        i -= 2;
    }

    void operator()(const standard_import& import){
        std::cout << indent() << "standard_import: " << import.file << std::endl;
    }
    
    void operator()(const import& import){
        std::cout << indent() << "import: " << import.file << std::endl;
    }
    
    void operator()(const function_declaration& function){
        std::cout << indent() << "function_declaration: " << function.name << std::endl;
        i += 2;
        std::cout << indent() << "return_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, function.return_type);
        i -= 2;
        std::cout << indent() << "parameters: " << std::endl;
        i += 2;
        for(auto& parameter : function.parameters){
            std::cout << indent() << "name: " << parameter.parameter_name << std::endl;
            std::cout << indent() << "type: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, parameter.parameter_type);
            i -= 2;
        }
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : function.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const foreach& foreach){
        std::cout << indent() << "foreach: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, foreach.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << foreach.variable_name << std::endl;
        std::cout << indent() << "from: " << foreach.from << std::endl;
        std::cout << indent() << "to: " << foreach.to << std::endl;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : foreach.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }
    
    void operator()(const foreach_in& foreach){
        std::cout << indent() << "foreach_in: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, foreach.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << foreach.variable_name << std::endl;
        std::cout << indent() << "array_name: " << foreach.array_name << std::endl;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : foreach.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const simple_type& type){
        std::cout << indent() << "simple_type: " << type.base_type << std::endl;
    }
    
    void operator()(const array_type& type){
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, type.base_type);
        i -= 2;
    }
    
    void operator()(const template_type& type){
        std::cout << indent() << "template_type: " << std::endl;
        i += 2;
        std::cout << indent() << "base_type: " << type.base_type << std::endl;
        std::cout << indent() << "template_types: " << std::endl;
        i += 2;
        for(auto& t : type.template_types){
            boost::apply_visitor(*this, t);
        }
        i -= 2;
        i -= 2;
    }
    
    void operator()(const pointer_type& type){
        std::cout << indent() << "pointer_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, type.base_type);
        i -= 2;
    }
};

} //end of x3_ast namespace

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::source_file,
    (std::vector<x3_ast::block>, blocks)
)

//***************

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::simple_type, 
    (bool, const_)
    (std::string, base_type)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::array_type, 
    (x3_ast::type, base_type)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::pointer_type, 
    (x3_ast::type, base_type)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::template_type, 
    (std::string, base_type)
    (std::vector<x3_ast::type>, template_types)
)

//***************

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::foreach_in, 
    (x3_ast::type, variable_type)
    (std::string, variable_name)
    (std::string, array_name)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::foreach, 
    (x3_ast::type, variable_type)
    (std::string, variable_name)
    (int, from)
    (int, to)
    (std::vector<x3_ast::instruction>, instructions)
)

//***************

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::function_declaration,
    (x3_ast::type, return_type)
    (std::string, name)
    (std::vector<x3_ast::function_parameter>, parameters)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::function_parameter, 
    (x3_ast::type, parameter_type)
    (std::string, parameter_name)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::import,
    (std::string, file)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::standard_import,
    (std::string, file)
)

#define ANNOTATE(Type)\
template <typename iterator_type, typename Attr, typename Context>\
inline void on_success(Type, const iterator_type& first, const iterator_type&, Attr& attr, Context const&){\
    auto& pos = first.get_position();\
    attr.pos.file = first.get_current_file();\
    attr.pos.line = pos.line;\
    attr.pos.column = pos.column;\
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-shift-op-parentheses"

namespace x3_grammar {
    typedef x3::identity<struct identifier> identifier_id;
    
    typedef x3::identity<struct source_file> source_file_id;

    typedef x3::identity<struct type> type_id;
    typedef x3::identity<struct simple_type> simple_type_id;
    typedef x3::identity<struct array_type> array_type_id;
    typedef x3::identity<struct pointer_type> pointer_type_id;
    typedef x3::identity<struct template_type> template_type_id;
    
    typedef x3::identity<struct instruction> instruction_id;
    typedef x3::identity<struct foreach> foreach_id;
    typedef x3::identity<struct foreach_in> foreach_in_id;

    typedef x3::identity<struct function_declaration> function_declaration_id;
    typedef x3::identity<struct function_parameter> function_parameter_id;
    typedef x3::identity<struct import> import_id;
    typedef x3::identity<struct standard_import> standard_import_id;

    x3::rule<identifier_id, std::string> const identifier("identifier");

    x3::rule<source_file_id, x3_ast::source_file> const source_file("source_file");

    x3::rule<type_id, x3_ast::type> const type("type");
    x3::rule<simple_type_id, x3_ast::simple_type> const simple_type("simple_type");
    x3::rule<array_type_id, x3_ast::array_type> const array_type("array_type");
    x3::rule<pointer_type_id, x3_ast::pointer_type> const pointer_type("pointer_type");
    x3::rule<template_type_id, x3_ast::template_type> const template_type("template_type");
    
    x3::rule<instruction_id, x3_ast::instruction> const instruction("instruction");
    x3::rule<foreach_id, x3_ast::foreach> const foreach("foreach");
    x3::rule<foreach_in_id, x3_ast::foreach_in> const foreach_in("foreach_in");

    x3::rule<function_declaration_id, x3_ast::function_declaration> const function_declaration("function_declaration");
    x3::rule<function_parameter_id, x3_ast::function_parameter> const function_parameter("function_parameter");
    x3::rule<standard_import_id, x3_ast::standard_import> const standard_import("standard_import");
    x3::rule<import_id, x3_ast::import> const import("import");

    ANNOTATE(import_id);
    ANNOTATE(standard_import_id);
    ANNOTATE(function_declaration_id);
    ANNOTATE(foreach_id);
    ANNOTATE(foreach_in_id);

    /* Utilities */
   
    auto const_ = 
            (x3::lit("const") > x3::attr(true))
        |   x3::attr(false);

    auto const identifier_def = 
                x3::lexeme[(x3::char_('_') >> *(x3::alnum | x3::char_('_')))]
            |   x3::lexeme[(x3::alpha >> *(x3::alnum | x3::char_('_')))]
            ;

    /* Base */ 
    
    auto const source_file_def = 
         *(
                standard_import
            |   import
            |   function_declaration
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
    
    auto const function_declaration_def = 
            type 
        >>  identifier
        >>  '(' 
        >>  function_parameter % ','
        >   ')'
        >   '{' 
        >   *instruction
        >   '}';

    auto const function_parameter_def =
            type
        >>  identifier;

    //*********************************************

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
    
    //*********************************************

    auto const instruction_def =
            foreach
        |   foreach_in;

    auto const foreach_def =
            x3::lit("foreach")
        >>  '('
        >>  type
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
        >>  type
        >>  identifier
        >>  "in"
        >>  identifier
        >>  ')'
        >>  '{'
        >>  *(instruction)
        >>  '}';

    //*********************************************
    
    auto const parser = x3::grammar(
        "eddi", 

        source_file = source_file_def,

        identifier = identifier_def,

        type = type_def,
        array_type = array_type_def,
        pointer_type = pointer_type_def,
        template_type = template_type_def,
        simple_type = simple_type_def,

        instruction = instruction_def,
        foreach = foreach_def,
        foreach_in = foreach_in_def,

        function_declaration = function_declaration_def, 
        function_parameter = function_parameter_def, 
        standard_import = standard_import_def,
        import = import_def);

    //********************************************

    auto const skipper = 
            x3::ascii::space
        |   ("/*" >> *(x3::char_ - "*/") >> "*/")
        |   ("//" >> *(x3::char_ - (x3::eol | x3::eoi)) >> (x3::eol | x3::eoi));

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

    x3_ast::source_file result;
    //boost::spirit::x3::ascii::space_type space;

    typedef std::string::iterator base_iterator_type;
    //typedef boost::spirit::classic::position_iterator2<base_iterator_type> pos_iterator_type;
    typedef extended_iterator<base_iterator_type> pos_iterator_type;

    pos_iterator_type it(file_contents.begin(), file_contents.end(), file, 1);
    pos_iterator_type end;

    try {
        bool r = x3::phrase_parse(it, end, parser, x3_grammar::skipper, result);

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
            <<  "got: \"" << std::string(e.first, e.last) << '"' << std::endl
            <<  "'" << e.first.get_currentline() << "'" << std::endl
            <<  std::setw(pos.column) << " " << "^- here" << std::endl;

        //TODO The position seems really off

        return false;
    }
}
