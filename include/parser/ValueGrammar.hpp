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

typedef lexer::lexer_type::iterator_type Iterator;
typedef lexer::SimpleLexer<lexer::lexer_type> Lexer;

struct ValueGrammar : qi::grammar<Iterator, ast::Value()> {
    ValueGrammar(const Lexer& lexer);
    
    qi::rule<Iterator, ast::Value()> value;
    qi::rule<Iterator, ast::Value()> primaryValue;
    qi::rule<Iterator, ast::Value()> unaryValue;
    qi::rule<Iterator, ast::NegatedValue()> negatedValue;
    qi::rule<Iterator, ast::ComposedValue()> additiveValue;
    qi::rule<Iterator, ast::ComposedValue()> multiplicativeValue;
    qi::rule<Iterator, ast::Value()> constant;
    qi::rule<Iterator, ast::Integer()> integer;
    qi::rule<Iterator, ast::Litteral()> litteral;
    qi::rule<Iterator, ast::VariableValue()> variable;
    qi::rule<Iterator, ast::ArrayValue()> arrayValue;
    qi::rule<Iterator, ast::FunctionCall()> functionCall;
};

} //end of parser

} //end of eddic

#endif
