#include "parser_x3/type_grammar.hpp"

namespace {

typedef x3::identity<struct type> type_id;
typedef x3::identity<struct simple_type> simple_type_id;
typedef x3::identity<struct array_type> array_type_id;
typedef x3::identity<struct pointer_type> pointer_type_id;
typedef x3::identity<struct template_type> template_type_id;

x3::rule<type_id, x3_ast::type> const type("type");
x3::rule<simple_type_id, x3_ast::simple_type> const simple_type("simple_type");
x3::rule<array_type_id, x3_ast::array_type> const array_type("array_type");
x3::rule<pointer_type_id, x3_ast::pointer_type> const pointer_type("pointer_type");
x3::rule<template_type_id, x3_ast::template_type> const template_type("template_type");

    #include "parser_x3/skipper_inc.hpp"
    #include "parser_x3/identifier_inc.hpp"

    auto const const_ = 
            (x3::lit("const") > x3::attr(true))
        |   x3::attr(false);

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
   
} //end of anonymous namespace

x3_grammar::type_parser_type x3_grammar::type_grammar_create(){

    return x3::skip(skipper)[x3::grammar(
        "eddi::type",
        type = type_def,
        simple_type = simple_type_def,
        template_type = template_type_def,
        array_type = array_type_def,
        pointer_type = pointer_type_def
        )];
}
