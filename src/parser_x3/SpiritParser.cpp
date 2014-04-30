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
#define BOOST_SPIRIT_X3_NO_RTTI

#include <boost/mpl/vector.hpp>
#include <boost/mpl/count.hpp>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>

//#include "GlobalContext.hpp"

#include "parser_x3/SpiritParser.hpp"

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

namespace x3_ast {

struct position {
    std::string file;               /*!< The source file */
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

struct integer_literal {
    int value;
};

struct integer_suffix_literal {
    int value;
    std::string suffix;
};

struct float_literal {
    double value;
};

struct string_literal {
    std::string value;
};

struct char_literal {
    char value;
};

struct variable_value {
    position pos;
    std::string variable_name;
};

typedef x3::variant<
            integer_literal,
            integer_suffix_literal,
            float_literal,
            string_literal,
            char_literal,
            variable_value
        > value;

//*****************************************

struct foreach;
struct while_;
struct do_while;
struct foreach_in;
struct variable_declaration;
struct struct_declaration;
struct array_declaration;
struct return_;
struct delete_;
struct if_;

typedef x3::variant<
        foreach,
        foreach_in,
        if_,
        while_,
        do_while,
        return_,
        delete_,
        variable_declaration,
        struct_declaration,
        array_declaration
    > instruction;

struct while_ {
    position pos;
    value condition;
    std::vector<instruction> instructions;
};

struct do_while {
    position pos;
    value condition;
    std::vector<instruction> instructions;
};

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

struct variable_declaration {
    position pos;
    type variable_type;
    std::string variable_name;
    boost::optional<x3_ast::value> value;
};

struct struct_declaration {
    position pos;
    type variable_type;
    std::string variable_name;
    std::vector<value> values;
};

struct array_declaration {
    position pos;
    type array_type;
    std::string array_name;
    value size;
};

struct return_ {
    position pos;
    int fake_;
    value return_value;
};

struct delete_ {
    position pos;
    int fake_;
    value value;
};

struct else_if {
    value condition;
    std::vector<instruction> instructions;
};

struct else_ {
    int fake_;
    std::vector<instruction> instructions;
};

struct if_ {
    value condition;
    std::vector<instruction> instructions;
    std::vector<else_if> else_ifs;
    boost::optional<x3_ast::else_> else_;
};

//*****************************************

struct function_parameter {
    type  parameter_type;
    std::string parameter_name;
};

struct template_function_declaration {
    position pos;
    std::vector<std::string> template_types;
    type return_type;
    std::string name;
    std::vector<function_parameter> parameters;
    std::vector<instruction> instructions;
};

struct global_variable_declaration {
    position pos;
    type variable_type;
    std::string variable_name;
    boost::optional<x3_ast::value> value;
};

struct global_array_declaration {
    position pos;
    type array_type;
    std::string array_name;
    value size;
};

struct standard_import {
    position pos;
    std::string file;
};

struct import {
    position pos;
    std::string file;
};

struct member_declaration {
    position pos;
    type type;
    std::string name;
};

typedef x3::variant<
        member_declaration,
        array_declaration,
        template_function_declaration
    > struct_block;

struct template_struct {
    std::vector<std::string> template_types;
    position pos;
    std::string name;
    boost::optional<type> parent_type;
    std::vector<struct_block> blocks;
};

typedef x3::variant<
        standard_import,
        import,
        template_struct,
        template_function_declaration,
        global_array_declaration,
        global_variable_declaration
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
    
    void operator()(const template_struct&){
        //TODO
    }
    
    void operator()(const template_function_declaration& function){
        std::cout << indent() << "template_function_declaration: " << function.name << std::endl;
        i += 2;
        std::cout << indent() << "template_types: ";
        for(auto& v : function.template_types) std::cout << v << ", ";
        std::cout << std::endl << indent() << "return_type: " << std::endl;
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

    void operator()(const while_& loop){
        std::cout << indent() << "while: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, loop.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : loop.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const do_while& loop){
        std::cout << indent() << "do while: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, loop.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : loop.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }

    void operator()(const if_& if_){
        std::cout << indent() << "if: " << std::endl;
        i += 2;
        std::cout << indent() << "condition: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, if_.condition);
        i -= 2;
        std::cout << indent() << "instructions: " << std::endl;
        i += 2;
        for(auto& instruction : if_.instructions){
            boost::apply_visitor(*this, instruction);
        }
        i -= 2;
        i -= 2;
    }
    
    void operator()(const return_& return_){
        std::cout << indent() << "return: " << std::endl;
        i += 2;
        std::cout << indent() << "value: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, return_.return_value);
        i -= 2;
        i -= 2;
    }
    
    void operator()(const delete_& delete_){
        std::cout << indent() << "delete: " << std::endl;
        i += 2;
        std::cout << indent() << "value: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, delete_.value);
        i -= 2;
        i -= 2;
    }
    
    void operator()(const variable_declaration& declaration){
        std::cout << indent() << "variable_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        if(declaration.value){
            std::cout << indent() << "value: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, *declaration.value);
            i -= 2;
        }
        i -= 2;
    }
    
    void operator()(const struct_declaration& declaration){
        std::cout << indent() << "struct_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        std::cout << indent() << "values: " << std::endl;
        i += 2;
        for(auto& v : declaration.values){
            boost::apply_visitor(*this, v);
        }
        i -= 2;
        i -= 2;
    }
    
    void operator()(const global_variable_declaration& declaration){
        std::cout << indent() << "global_variable_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "variable_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.variable_type);
        i -= 2;
        std::cout << indent() << "variable_name: " << declaration.variable_name << std::endl;
        if(declaration.value){
            std::cout << indent() << "value: " << std::endl;
            i += 2;
            boost::apply_visitor(*this, *declaration.value);
            i -= 2;
        }
        i -= 2;
    }
    
    void operator()(const global_array_declaration& declaration){
        std::cout << indent() << "global_array_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.array_type);
        i -= 2;
        std::cout << indent() << "array_name: " << declaration.array_name << std::endl;
        std::cout << indent() << "size: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.size);
        i -= 2;
        i -= 2;
    }
    
    void operator()(const array_declaration& declaration){
        std::cout << indent() << "array_declaration: " << std::endl;
        i += 2;
        std::cout << indent() << "array_type: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.array_type);
        i -= 2;
        std::cout << indent() << "array_name: " << declaration.array_name << std::endl;
        std::cout << indent() << "size: " << std::endl;
        i += 2;
        boost::apply_visitor(*this, declaration.size);
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

    void operator()(const integer_literal& integer){
        std::cout << indent() << "integer_literal: " << integer.value << std::endl;
    }
    
    void operator()(const integer_suffix_literal& integer){
        std::cout << indent() << "integer_suffix_literal: " << integer.value << integer.suffix << std::endl;
    }

    void operator()(const float_literal& integer){
        std::cout << indent() << "float_literal: " << integer.value << std::endl;
    }
    
    void operator()(const string_literal& integer){
        std::cout << indent() << "string_literal: " << integer.value << std::endl;
    }
    
    void operator()(const char_literal& integer){
        std::cout << indent() << "char_literal: " << integer.value << std::endl;
    }
    
    void operator()(const variable_value& integer){
        std::cout << indent() << "variable_value: " << integer.variable_name << std::endl;
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
    x3_ast::integer_literal, 
    (int, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::integer_suffix_literal, 
    (int, value)
    (std::string, suffix)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::float_literal, 
    (double, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::string_literal, 
    (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::char_literal, 
    (char, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::variable_value, 
    (std::string, variable_name)
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

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::while_, 
    (x3_ast::value, condition)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::do_while, 
    (std::vector<x3_ast::instruction>, instructions)
    (x3_ast::value, condition)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::variable_declaration, 
    (x3_ast::type, variable_type)
    (std::string, variable_name)
    (boost::optional<x3_ast::value>, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::struct_declaration, 
    (x3_ast::type, variable_type)
    (std::string, variable_name)
    (std::vector<x3_ast::value>, values)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::array_declaration, 
    (x3_ast::type, array_type)
    (std::string, array_name)
    (x3_ast::value, size)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::return_, 
    (int, fake_)
    (x3_ast::value, return_value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::delete_, 
    (int, fake_)
    (x3_ast::value, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::else_if, 
    (x3_ast::value, condition)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::else_, 
    (int, fake_)
    (std::vector<x3_ast::instruction>, instructions)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::if_, 
    (x3_ast::value, condition)
    (std::vector<x3_ast::instruction>, instructions)
    (std::vector<x3_ast::else_if>, else_ifs)
    (boost::optional<x3_ast::else_>, else_)
)

//***************

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::template_function_declaration,
    (std::vector<std::string>, template_types)
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
    x3_ast::global_variable_declaration, 
    (x3_ast::type, variable_type)
    (std::string, variable_name)
    (boost::optional<x3_ast::value>, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::global_array_declaration, 
    (x3_ast::type, array_type)
    (std::string, array_name)
    (x3_ast::value, size)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::import,
    (std::string, file)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::standard_import,
    (std::string, file)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::member_declaration,
    (x3_ast::type, type)
    (std::string, name)
)

BOOST_FUSION_ADAPT_STRUCT(
    x3_ast::template_struct,
    (std::vector<std::string>, template_types)
    (std::string, name)
    (boost::optional<x3_ast::type>, parent_type)
    (std::vector<x3_ast::struct_block>, blocks)
)

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

    type_parser_type const type_grammar = 
        x3::skip(skipper)[x3::grammar(
            "eddi::type",
            type = type_def,
            simple_type = simple_type_def,
            template_type = template_type_def,
            array_type = array_type_def,
            pointer_type = pointer_type_def
        )];
    
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

    value_parser_type const value_grammar = 
        x3::skip(skipper)[x3::grammar(
            "eddi::value",
            value = value_def,
            integer_literal = integer_literal_def,
            integer_suffix_literal = integer_suffix_literal_def,
            float_literal = float_literal_def,
            char_literal = char_literal_def,
            string_literal = string_literal_def,
            variable_value = variable_value_def
        )];
    
    /* Instructions */

    auto const instruction_def =
            if_
        |   foreach
        |   foreach_in
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

    instruction_parser_type const instruction_grammar =
        x3::skip(skipper)[x3::grammar(
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
