//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/ValueGrammar.hpp"
#include "lexer/adapttokens.hpp"

using namespace eddic;

parser::ValueGrammar::ValueGrammar(const lexer::Lexer& lexer) : ValueGrammar::base_type(value, "Value Grammar") {
    /* Match operators into symbols */
    //TODO Find a way to avoid duplication of these things
    additive_op.add
        ("+", ast::Operator::ADD)
        ("-", ast::Operator::SUB)
        ;

    multiplicative_op.add
        ("/", ast::Operator::DIV)
        ("*", ast::Operator::MUL)
        ("%", ast::Operator::MOD)
        ;
    
    relational_op.add
        (">=", ast::Operator::GREATER_EQUALS) 
        (">", ast::Operator::GREATER) 
        ("<=", ast::Operator::LESS_EQUALS) 
        ("<", ast::Operator::LESS) 
        ("!=", ast::Operator::NOT_EQUALS) 
        ("==", ast::Operator::EQUALS) 
        ;
    
    logical_and_op.add
        ("&&", ast::Operator::AND) 
        ;
    
    logical_or_op.add
        ("||", ast::Operator::OR) 
        ;

    //TODO Use unary_op symbols and use a UnaryValue to represent plus and minus for a value

    /* Define values */ 

    value = logicalOrValue.alias();
    
    logicalOrValue %=
            logicalAndValue
        >>  *(qi::adapttokens[logical_or_op] > logicalAndValue);  
    
    logicalAndValue %=
            relationalValue
        >>  *(qi::adapttokens[logical_and_op] > relationalValue);  
   
    relationalValue %=
            additiveValue
        >>  *(qi::adapttokens[relational_op] > additiveValue);  
    
    additiveValue %=
            multiplicativeValue
        >>  *(qi::adapttokens[additive_op] > multiplicativeValue);
   
    multiplicativeValue %=
            unaryValue
        >>  *(qi::adapttokens[multiplicative_op] > unaryValue);
    
    unaryValue %= 
            negatedValue
        |   plusValue
        |   primaryValue;
   
    negatedValue = 
            lexer.subtraction
         >> primaryValue;
  
    plusValue %=
            lexer.addition
         >> primaryValue;
    
    primaryValue = 
            constant 
        |   functionCall
        |   arrayValue
        |   variable 
        |   true_
        |   false_
        |   (lexer.left_parenth >> value > lexer.right_parenth);
    
    true_ %= 
            qi::eps
        >>  lexer.true_;
    
    false_ %= 
            qi::eps
        >>  lexer.false_;

    integer %= 
            qi::eps 
        >>  lexer.integer;
   
    variable %= 
            qi::eps
        >>  lexer.word;
   
    arrayValue %=
            lexer.word
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket;
    
    litteral %= 
            qi::eps 
        >> lexer.litteral;

    constant %= 
            integer 
        |   litteral;
    
    functionCall %=
            lexer.word
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >>  lexer.right_parenth;
}
