//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/ValueGrammar.hpp"
#include "lexer/adapttokens.hpp"
#include "lexer/position.hpp"

using namespace eddic;

parser::ValueGrammar::ValueGrammar(const lexer::Lexer& lexer, const lexer::pos_iterator_type& position_begin) : 
        ValueGrammar::base_type(value, "Value Grammar"),
        type(lexer, position_begin),
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
            qi::position(position_begin)
        >>  logicalAndValue
        >>  *(qi::adapttokens[logical_or_op] > logicalAndValue);  
    
    logicalAndValue %=
            qi::position(position_begin)
        >>  relationalValue
        >>  *(qi::adapttokens[logical_and_op] > relationalValue);  
   
    relationalValue %=
            qi::position(position_begin)
        >>  additiveValue
        >>  *(qi::adapttokens[relational_op] > additiveValue);  
    
    additiveValue %=
            qi::position(position_begin)
        >>  multiplicativeValue
        >>  *(qi::adapttokens[additive_op] > multiplicativeValue);
   
    multiplicativeValue %=
            qi::position(position_begin)
        >>  unaryValue
        >>  *(qi::adapttokens[multiplicative_op] > unaryValue);
    
    unaryValue %= 
            negatedValue
        |   plusValue
        |   castValue
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

    castValue %=
            qi::position(position_begin)
        >>  lexer.left_parenth
        >>  type.type
        >>  lexer.right_parenth
        >>  primaryValue;
    
    primaryValue = 
            assignment
        |   integer_suffix
        |   integer
        |   float_
        |   litteral
        |   builtin_operator
        |   functionCall
        |   prefix_operation
        |   suffix_operation
        |   struct_value
        |   arrayValue
        |   variable_value
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

    integer_suffix %=
            qi::eps
        >>  lexer.integer
        >>  lexer.identifier;

    float_ %= 
            qi::eps 
        >>  lexer.float_;
   
    variable_value %= 
            qi::position(position_begin)
        >>  lexer.identifier;
    
    struct_value %= 
            qi::position(position_begin)
        >>  lexer.identifier
        >>  lexer.dot
        >>  lexer.identifier;
   
    arrayValue %=
            qi::position(position_begin)
        >>  lexer.identifier
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
            qi::position(position_begin)
        >>  qi::adapttokens[builtin_op]
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >   lexer.right_parenth;
    
    functionCall %=
            qi::position(position_begin)
        >>  lexer.identifier
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >   lexer.right_parenth;
    
    assignment %= 
            qi::position(position_begin)
        >>  lexer.identifier 
        >>  lexer.assign 
        >>  value;
    
    prefix_operation %=
            qi::position(position_begin)
        >>  qi::adapttokens[prefix_op]
        >>  lexer.identifier;

    suffix_operation %=
            qi::position(position_begin)
        >>  lexer.identifier
        >>  qi::adapttokens[suffix_op];
}
