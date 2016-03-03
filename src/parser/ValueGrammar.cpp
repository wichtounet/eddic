//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "parser/ValueGrammar.hpp"
#include "parser/Utils.hpp"

#include "lexer/adapttokens.hpp"
#include "lexer/position.hpp"
#include <boost/spirit/include/phoenix.hpp>

using namespace eddic;

parser::ValueGrammar::ValueGrammar(const lexer::StaticLexer& lexer) :
        ValueGrammar::base_type(start, "_value Grammar"),
        type(lexer)
{
    auto local_begin     = qi::lazy ( boost::phoenix::construct<qi::position>(qi::_a, qi::_b) );
    auto inherited_begin = qi::lazy ( boost::phoenix::construct<qi::position>(qi::_r1, qi::_r2) );

    start %= qi::eps [ qi::_a = qi::_r1, qi::_b = qi::_r2 ] >> value;

    /* Match operators into symbols */

#if 0 //Disable for X3

    unary_op.add
        ("+", ast::Operator::ADD)
        ("-", ast::Operator::SUB)
        ("!", ast::Operator::NOT)
        ("*", ast::Operator::STAR)
        ("&", ast::Operator::ADDRESS)
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

    postfix_op.add
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
        ("<=>",  ast::Operator::SWAP)
        ("+=", ast::Operator::ADD)
        ("-=", ast::Operator::SUB)
        ("/=", ast::Operator::DIV)
        ("*=", ast::Operator::MUL)
        ("%=", ast::Operator::MOD)
        ;

    /* Terminal values */

    new_array %=
            local_begin
        >>  lexer.new_
        >>  type.type
        >>  lexer.left_bracket
        >>  value
        >>  lexer.right_bracket;

    new_ %=
            local_begin
        >>  lexer.new_
        >>  type.type
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >>  lexer.right_parenth;

    variable_value %=
            qi::eps
        >>
            (
                lexer.this_
            |   lexer.identifier
            )
    ;

    auto null = boost::spirit::qi::as<ast::Null>()[(
            qi::eps
        >>  lexer.null
    )];

    auto true_ = boost::spirit::qi::as<ast::True>()[(
            qi::eps
        >>  lexer.true_
    )];

    auto false_ = boost::spirit::qi::as<ast::False>()[(
            qi::eps
        >>  lexer.false_
    )];

    auto integer = boost::spirit::qi::as<ast::Integer>()[(
            qi::eps
        >>  lexer.integer
    )];

    auto integer_suffix = boost::spirit::qi::as<ast::IntegerSuffix>()[(
            lexer.integer
        >>  lexer.identifier
    )];

    auto float_ = boost::spirit::qi::as<ast::Float>()[(
            qi::eps
        >>  lexer.float_
    )];

    auto string_literal = boost::spirit::qi::as<ast::Literal>()[(
            qi::eps
        >> lexer.string_literal
    )];

    auto char_literal = boost::spirit::qi::as<ast::CharLiteral>()[(
            qi::eps
        >> lexer.char_literal
    )];

    /* Define values */

    primary_value =
            integer_suffix
        |   integer
        |   float_
        |   string_literal
        |   char_literal
        |   builtin_operator
        |   function_call(qi::_a, qi::_b)
        |   new_array
        |   new_
        |   variable_value
        |   null
        |   (lexer.left_parenth >> value >> lexer.right_parenth);

    call_value %=
            lexer.identifier
        >>  -(
                    qi::omit[lexer.less]
                >>  type >> *(lexer.comma > type)
                >>  qi::omit[lexer.greater]
            )
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >>  lexer.right_parenth;

    postfix_expression %=
            inherited_begin
        >>  primary_value
        >>  +(
                         lexer.left_bracket
                     >>  boost::spirit::attr(ast::Operator::BRACKET)
                     >>  value
                     >>  lexer.right_bracket
                |
                         lexer.dot
                     >>  boost::spirit::attr(ast::Operator::CALL)
                     >>  call_value
                |
                         lexer.dot
                     >>  boost::spirit::attr(ast::Operator::DOT)
                     >>  qi::as<std::string>()[((lexer.identifier))]
                |
                    qi::adapttokens[postfix_op]
            );

    prefix_operation %=
            inherited_begin
        >>  qi::adapttokens[prefix_op]
        >>  unary_expression;

    unary_operation %=
            local_begin
        >>  qi::adapttokens[unary_op]
        >>  cast_expression;

    unary_expression %=
            postfix_expression(qi::_a, qi::_b)
        |   prefix_operation(qi::_a, qi::_b)
        |   unary_operation
        |   primary_value;

    cast_value %=
            local_begin
        >>  lexer.left_parenth
        >>  type.type
        >>  lexer.right_parenth
        >>  cast_expression;

    cast_expression %=
            cast_value
        |   unary_expression;

    multiplicative_value %=
            local_begin
        >>  cast_expression
        >>  *(qi::adapttokens[multiplicative_op] >> cast_expression);

    additive_value %=
            local_begin
        >>  multiplicative_value
        >>  *(qi::adapttokens[additive_op] >> multiplicative_value);

    relational_value %=
            local_begin
        >>  additive_value
        >>  *(qi::adapttokens[relational_op] >> additive_value);

    logicalAnd_value %=
            local_begin
        >>  relational_value
        >>  *(qi::adapttokens[logical_and_op] >> relational_value);

    logicalOr_value %=
            local_begin
        >>  logicalAnd_value
        >>  *(qi::adapttokens[logical_or_op] >> logicalAnd_value);

    ternary %=
            local_begin
        >>  logicalOr_value
        >>  lexer.question_mark
        >>  conditional_expression
        >>  lexer.double_dot
        >>  conditional_expression;

    conditional_expression =
            ternary
         |  logicalOr_value;

    assignment_expression %=
            assignment(qi::_a, qi::_b)
        |   conditional_expression;

    assignment %=
            inherited_begin
        >>  unary_expression
        >>  qi::adapttokens[assign_op]
        >>  assignment_expression;

    value = assignment_expression.alias();

    function_call %=
            inherited_begin
        >>  lexer.identifier
        >>  -(
                    qi::omit[lexer.less]
                >>  type >> *(lexer.comma > type)
                >>  qi::omit[lexer.greater]
            )
        >>  lexer.left_parenth
        >>  -( value >> *( lexer.comma > value))
        >>  lexer.right_parenth;

    builtin_operator %=
            local_begin
        >>  qi::adapttokens[builtin_op]
        >>  lexer.left_parenth
        >>  value >> *( lexer.comma > value)
        >>  lexer.right_parenth;

#endif

}
