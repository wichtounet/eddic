#ifndef PARSER_AST_H
#define PARSER_AST_H

namespace x3 = boost::spirit::x3;

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

#endif
