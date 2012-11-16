//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VALUE_GRAMMAR_H
#define VALUE_GRAMMAR_H

#include "boost_cfg.hpp"
#include <boost/spirit/include/qi.hpp>
#include "lexer/SpiritLexer.hpp"

#include "ast/Value.hpp"
#include "parser/TypeGrammar.hpp"

namespace qi = boost::spirit::qi;

namespace eddic {

namespace parser {

/*!
 * \class ValueGrammar
 * \brief Grammar representing values in EDDI language.
 */
struct ValueGrammar : qi::grammar<lexer::Iterator, ast::Value()> {
    struct cast_impl {
        template < typename A >
        struct result { typedef ast::OperationValue type; };

        ast::OperationValue operator()(const ast::Value& arg) const {
            ast::OperationValueVariant var = arg;
            ast::OperationValue value = var;
            return value;
        }
        
        ast::OperationValue operator()(const std::string& arg) const {
            ast::OperationValueVariant var = arg;
            ast::OperationValue value = var;
            return value;
        }
    };

    boost::phoenix::function<cast_impl> cast;

    ValueGrammar(const lexer::Lexer& lexer, const lexer::pos_iterator_type& position_begin);
    
    qi::rule<lexer::Iterator, ast::Assignment()> assignment;
    qi::rule<lexer::Iterator, ast::Ternary()> ternary;
    qi::rule<lexer::Iterator, ast::Value()> value;
    qi::rule<lexer::Iterator, ast::Value()> primary_value;
    qi::rule<lexer::Iterator, ast::Cast()> cast_value;
    qi::rule<lexer::Iterator, ast::PrefixOperation()> negated_constant_value;
    qi::rule<lexer::Iterator, ast::Value()> conditional_expression;
    
    qi::rule<lexer::Iterator, ast::Expression()> postfix_expression;
    qi::rule<lexer::Iterator, ast::Expression()> postfix_expression_helper;

    qi::rule<lexer::Iterator, ast::Expression()> additive_value;
    qi::rule<lexer::Iterator, ast::Expression()> multiplicative_value;
    qi::rule<lexer::Iterator, ast::Expression()> relational_value;
    qi::rule<lexer::Iterator, ast::Expression()> logicalAnd_value;
    qi::rule<lexer::Iterator, ast::Expression()> logicalOr_value;

    qi::rule<lexer::Iterator, ast::Value()> constant;
    qi::rule<lexer::Iterator, ast::Integer()> integer;
    qi::rule<lexer::Iterator, ast::IntegerSuffix()> integer_suffix;
    qi::rule<lexer::Iterator, ast::Float()> float_;
    qi::rule<lexer::Iterator, ast::Literal()> string_literal;
    qi::rule<lexer::Iterator, ast::CharLiteral()> char_literal;
    qi::rule<lexer::Iterator, ast::VariableValue()> variable_value;
    qi::rule<lexer::Iterator, ast::BuiltinOperator()> builtin_operator;
    qi::rule<lexer::Iterator, ast::FunctionCall()> function_call;
    qi::rule<lexer::Iterator, ast::MemberFunctionCall()> member_function_call;
    qi::rule<lexer::Iterator, ast::True()> true_;
    qi::rule<lexer::Iterator, ast::False()> false_;
    qi::rule<lexer::Iterator, ast::Null()> null;
    qi::rule<lexer::Iterator, ast::New()> new_;
    qi::rule<lexer::Iterator, ast::NewArray()> new_array;
    qi::rule<lexer::Iterator, ast::PrefixOperation()> prefix_operation;
    qi::rule<lexer::Iterator, ast::PrefixOperation()> unary_operation;
    
    qi::rule<lexer::Iterator, ast::Value()> assignment_expression;
    qi::rule<lexer::Iterator, ast::Value()> unary_expression;
    qi::rule<lexer::Iterator, ast::Value()> cast_expression;
    
    qi::rule<lexer::Iterator, ast::PostfixOperation()> old_postfix_operation;
    qi::rule<lexer::Iterator, ast::Value()> old_postfix_expression;

    //Versions with cast to ast::OperationValue
    qi::rule<lexer::Iterator, ast::OperationValue()> limited_value;
    qi::rule<lexer::Iterator, ast::OperationValue()> limited_string_literal;
    qi::rule<lexer::Iterator, ast::OperationValue()> limited_cast_expression;
    qi::rule<lexer::Iterator, ast::OperationValue()> limited_additive_value;
    qi::rule<lexer::Iterator, ast::OperationValue()> limited_multiplicative_value;
    qi::rule<lexer::Iterator, ast::OperationValue()> limited_relational_value;
    qi::rule<lexer::Iterator, ast::OperationValue()> limited_logicalAnd_value;

    /* Operators */

    qi::symbols<char, ast::Operator> multiplicative_op;
    qi::symbols<char, ast::Operator> additive_op;
    qi::symbols<char, ast::Operator> relational_op;
    qi::symbols<char, ast::Operator> logical_and_op;
    qi::symbols<char, ast::Operator> logical_or_op;
    qi::symbols<char, ast::Operator> postfix_op;
    qi::symbols<char, ast::Operator> prefix_op;
    qi::symbols<char, ast::Operator> assign_op;
    qi::symbols<char, ast::Operator> unary_op;

    qi::symbols<char, ast::BuiltinType> builtin_op;
    
    TypeGrammar type;
    
    const lexer::pos_iterator_type& position_begin;
};

} //end of parser

} //end of eddic

#endif
