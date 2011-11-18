//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/BooleanGrammar.hpp"

using namespace eddic;

BooleanGrammar::BooleanGrammar(const Lexer& lexer) : BooleanGrammar::base_type(condition, "Boolean Grammar"), value(lexer) {
    true_ %= 
            qi::eps
        >>  lexer.true_;
    
    false_ %= 
            qi::eps
        >>  lexer.false_;

    binary_condition %=
            value
        >>  (
                lexer.greater_equals
            |   lexer.greater
            |   lexer.less_equals
            |   lexer.less
            |   lexer.not_equals
            |   lexer.equals
            )
        >>   value;

    condition %= 
            true_ 
        |   false_ 
        |   binary_condition;
}
