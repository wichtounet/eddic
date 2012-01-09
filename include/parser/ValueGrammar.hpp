//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VALUE_GRAMMAR_H
#define VALUE_GRAMMAR_H

#include <boost/spirit/include/qi.hpp>
#include "lexer/SpiritLexer.hpp"

#include "ast/Value.hpp"

namespace qi = boost::spirit::qi;

namespace eddic {

namespace parser {

/*!
 * \class ValueGrammar
 * \brief Grammar representing values in EDDI language.
 */
struct ValueGrammar : qi::grammar<lexer::Iterator, ast::Value()> {
    ValueGrammar(const lexer::Lexer& lexer);
    
    qi::rule<lexer::Iterator, ast::Value()> value;
    qi::rule<lexer::Iterator, ast::Value()> primaryValue;
    qi::rule<lexer::Iterator, ast::Value()> unaryValue;
    qi::rule<lexer::Iterator, ast::Minus()> negatedValue;
    qi::rule<lexer::Iterator, ast::Plus()> plusValue;
    qi::rule<lexer::Iterator, ast::ComposedValue()> additiveValue;
    qi::rule<lexer::Iterator, ast::ComposedValue()> multiplicativeValue;
    qi::rule<lexer::Iterator, ast::Value()> constant;
    qi::rule<lexer::Iterator, ast::Integer()> integer;
    qi::rule<lexer::Iterator, ast::Litteral()> litteral;
    qi::rule<lexer::Iterator, ast::VariableValue()> variable;
    qi::rule<lexer::Iterator, ast::ArrayValue()> arrayValue;
    qi::rule<lexer::Iterator, ast::FunctionCall()> functionCall;
};

} //end of parser

} //end of eddic

#endif
