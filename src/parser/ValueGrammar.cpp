//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/ValueGrammar.hpp"
#include "lexer/adapttokens.hpp"

using namespace eddic;

parser::ValueGrammar::ValueGrammar(const lexer::Lexer& lexer, const lexer::pos_iterator_type& position_begin) : 
        ValueGrammar::base_type(value, "Value Grammar"),
        position_begin(position_begin){

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
    
    suffix_op.add
        ("++", ast::Operator::INC)
        ("--", ast::Operator::DEC)
        ;
    
    prefix_op.add
        ("++", ast::Operator::INC)
        ("--", ast::Operator::DEC)
        ;
    
    builtin_op.add
        ("size", ast::BuiltinType::SIZE)
        ("length", ast::BuiltinType::LENGTH)
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
   
    negatedConstantValue = 
            lexer.subtraction
         >> integer;
  
    plusValue %=
            lexer.addition
         >> primaryValue;
    
    primaryValue = 
            assignment
        |   integer
        |   litteral
        |   builtin_operator
        |   functionCall
        |   prefix_operation
        |   suffix_operation
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
        >>  lexer.identifier;
   
    arrayValue %=
            lexer.identifier
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket;
    
    litteral %= 
            qi::eps 
        >> lexer.litteral;

    constant = 
            negatedConstantValue
        |   integer 
        |   litteral;
   
    builtin_operator %=
            qi::adapttokens[builtin_op]
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >   lexer.right_parenth;
    
    functionCall %=
            lexer.identifier
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >   lexer.right_parenth;
    
    assignment %= 
            lexer.identifier 
        >>  lexer.assign 
        >>  value;
    
    prefix_operation %=
            qi::adapttokens[prefix_op]
        >>  lexer.identifier;

    suffix_operation %=
            lexer.identifier
        >>  qi::adapttokens[suffix_op];
}
