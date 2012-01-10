//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/BooleanGrammar.hpp"
#include "lexer/adapttokens.hpp"

using namespace eddic;

parser::BooleanGrammar::BooleanGrammar(const lexer::Lexer& lexer) : BooleanGrammar::base_type(condition, "Boolean Grammar"), value(lexer) {
    logical_binary_op.add
        (">=", ast::Operator::GREATER_EQUALS) 
        (">", ast::Operator::GREATER) 
        ("<=", ast::Operator::LESS_EQUALS) 
        ("<", ast::Operator::LESS) 
        ("!=", ast::Operator::NOT_EQUALS) 
        ("==", ast::Operator::EQUALS) 
        ;

    true_ %= 
            qi::eps
        >>  lexer.true_;
    
    false_ %= 
            qi::eps
        >>  lexer.false_;

    binary_condition %=
            value
        >>  qi::adapttokens[logical_binary_op]
        >>  value;

    condition %= 
            true_ 
        |   false_ 
        |   binary_condition;
}
