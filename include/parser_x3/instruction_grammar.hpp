#ifndef PARSER_INSTRUCTION_GRAMMAR_H
#define PARSER_INSTRUCTION_GRAMMAR_H

#include <boost/spirit/home/x3.hpp>

#include "parser_x3/ast.hpp"
#include "parser_x3/iterator.hpp"
    
namespace x3_grammar {

using instruction_parser_type = x3::any_parser<pos_iterator_type, x3_ast::instruction>;

instruction_parser_type instruction_grammar_create();

auto const instruction_grammar = instruction_grammar_create();

}

#endif
