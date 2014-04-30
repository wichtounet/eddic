#ifndef PARSER_VALUE_GRAMMAR_H
#define PARSER_VALUE_GRAMMAR_H

#include <boost/spirit/home/x3.hpp>

#include "parser_x3/ast.hpp"
#include "parser_x3/iterator.hpp"
    
namespace x3_grammar {

using value_parser_type = x3::any_parser<pos_iterator_type, x3_ast::value>;

value_parser_type value_grammar_create();

auto const value_grammar = value_grammar_create();

}

#endif
