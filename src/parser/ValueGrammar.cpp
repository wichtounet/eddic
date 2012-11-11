//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "parser/ValueGrammar.hpp"
#include "parser/Utils.hpp"

#include "lexer/adapttokens.hpp"
#include "lexer/position.hpp"

using namespace eddic;

parser::ValueGrammar::ValueGrammar(const lexer::Lexer& lexer, const lexer::pos_iterator_type& position_begin) : 
        ValueGrammar::base_type(value, "Value Grammar"),
        type(lexer, position_begin),
        position_begin(position_begin){

    /* Match operators into symbols */
    
    unary_op.add
        ("+", ast::Operator::ADD)
        ("-", ast::Operator::SUB)
        ("!", ast::Operator::NOT)
   //   ("*", ast::Operator::STAR)
        ;

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
    
    assign_op.add
        ("=",  ast::Operator::ASSIGN)
        ("+=", ast::Operator::ADD)
        ("-=", ast::Operator::SUB)
        ("/=", ast::Operator::DIV)
        ("*=", ast::Operator::MUL)
        ("%=", ast::Operator::MOD)
        ;

    /* Terminal values */
    
    new_array %=
            qi::position(position_begin)
        >>  lexer.new_
        >>  type.type
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket;

    new_ %=
            qi::position(position_begin)
        >>  lexer.new_
        >>  type.type
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >>  lexer.right_parenth;
    
    null %= 
            qi::eps
        >>  lexer.null;
    
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
    
    string_literal %= 
            qi::eps 
        >> lexer.string_literal;
    
    char_literal %= 
            qi::eps 
        >> lexer.char_literal;

    constant = 
            negated_constant_value
        |   integer 
        |   string_literal
        |   char_literal;

    /* Define values */ 
    
    primary_value = 
            assignment
        |   integer_suffix
        |   integer
        |   float_
        |   string_literal
        |   char_literal
        |   builtin_operator
        |   member_function_call
        |   function_call
        |   prefix_operation
        |   new_array
        |   new_
        |   suffix_operation
        |   member_value
        |   array_value
        |   variable_value
        |   dereference_value
        |   null
        |   true_
        |   false_
        |   (lexer.left_parenth >> value > lexer.right_parenth);

    suffix_operation %=
            qi::position(position_begin)
        >>  (
                    array_value
                |   variable_value
            )
        >>  qi::adapttokens[suffix_op];

    prefix_operation %=
            qi::position(position_begin)
        >>  qi::adapttokens[prefix_op]
        >>  (
                    array_value
                |   variable_value
            );

    castValue %=
            qi::position(position_begin)
        >>  lexer.left_parenth
        >>  type.type
        >>  lexer.right_parenth
        >>  primary_value;
   
    unaryValue %= 
            negated_constant_value
        |   castValue    
        |   unary_value
        |   primary_value;
    
    negated_constant_value = 
            qi::adapttokens[unary_op]
         >> integer;

    unary_value %=
            qi::adapttokens[unary_op] 
        >   primary_value
            ;
    
    multiplicativeValue %=
            qi::position(position_begin)
        >>  unaryValue
        >>  *(qi::adapttokens[multiplicative_op] > unaryValue);
    
    additiveValue %=
            qi::position(position_begin)
        >>  multiplicativeValue
        >>  *(qi::adapttokens[additive_op] > multiplicativeValue);
   
    relationalValue %=
            qi::position(position_begin)
        >>  additiveValue
        >>  *(qi::adapttokens[relational_op] > additiveValue);  
    
    logicalAndValue %=
            qi::position(position_begin)
        >>  relationalValue
        >>  *(qi::adapttokens[logical_and_op] > relationalValue);  
    
    logicalOrValue %=
            qi::position(position_begin)
        >>  logicalAndValue
        >>  *(qi::adapttokens[logical_or_op] > logicalAndValue);  

    ternary %=
            qi::position(position_begin)
        >>  logicalOrValue 
        >>  lexer.question_mark
        >>  conditional_expression 
        >>  lexer.double_dot
        >>  conditional_expression;

    conditional_expression =
            ternary
         |  logicalOrValue;

    value = conditional_expression.alias();
    
    member_value %= 
            qi::position(position_begin)
        >>  
            (
                    array_value
                |   variable_value
            )
        >>  +(
                    lexer.dot
                >>  lexer.identifier
             );
   
    variable_value %= 
            qi::position(position_begin)
        >>  
            (
                    lexer.this_
                |   lexer.identifier
            );
   
    array_value %=
            qi::position(position_begin)
        >>  (
                    //member_value
                /*|*/   variable_value
            )
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket;
   
    dereference_value %= 
            qi::position(position_begin)
        >>  qi::omit[lexer.multiplication]
        >>  (
                    member_value
                |   array_value
                |   variable_value
            );

    function_call %=
            qi::position(position_begin)
        >>  lexer.identifier
        >>  -(
                    qi::omit[lexer.less]
                >>  type >> *(lexer.comma > type)
                >>  qi::omit[lexer.greater]
            )
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >   lexer.right_parenth;
    
    member_function_call %=
            qi::position(position_begin)
        >>  
            (
                variable_value
            )
        >>  lexer.dot
        >>  lexer.identifier
        >>  -(
                    qi::omit[lexer.less]
                >>  type >> *(lexer.comma > type)
                >>  qi::omit[lexer.greater]
            )
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >   lexer.right_parenth;
   
    builtin_operator %=
            qi::position(position_begin)
        >>  qi::adapttokens[builtin_op]
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >   lexer.right_parenth;
    
    assignment %= 
            qi::position(position_begin)
        >>  (
                    member_value
                |   array_value
                |   variable_value
                |   dereference_value
            )
        >>  qi::adapttokens[assign_op]
        >>  value;

    //Configure debugging

    DEBUG_RULE(assignment);
    DEBUG_RULE(suffix_operation);
    DEBUG_RULE(prefix_operation);
    DEBUG_RULE(builtin_operator);
    DEBUG_RULE(array_value);
    DEBUG_RULE(variable_value);
    DEBUG_RULE(dereference_value);
    DEBUG_RULE(function_call);
    DEBUG_RULE(primary_value);
    DEBUG_RULE(ternary);
    DEBUG_RULE(constant);
    DEBUG_RULE(string_literal);
    DEBUG_RULE(char_literal);
}
