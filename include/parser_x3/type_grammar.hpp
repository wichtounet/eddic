#ifndef PARSER_TYPE_GRAMMAR_H
#define PARSER_TYPE_GRAMMAR_H

#include <boost/spirit/home/x3.hpp>

#include "parser_x3/ast.hpp"
#include "parser_x3/iterator.hpp"
    
namespace x3_grammar {

using type_parser_type = x3::any_parser<pos_iterator_type, x3_ast::type>;

type_parser_type type_grammar_create();

auto const type_grammar = type_grammar_create();

}

#endif
