//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/TypeGrammar.hpp"

using namespace eddic;

parser::TypeGrammar::TypeGrammar(const lexer::Lexer& lexer, const lexer::pos_iterator_type& position_begin) : 
        TypeGrammar::base_type(type, "Type Grammar"),
        position_begin(position_begin){

    arrayType %=
            qi::eps
        >>  lexer.identifier
        >>  lexer.left_bracket
        >>  lexer.right_bracket;

    simpleType %=
            qi::eps
        >>  lexer.identifier;

    type %=
            arrayType
        |   simpleType;
}
