//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOLEAN_GRAMMAR_H
#define BOOLEAN_GRAMMAR_H

#include <boost/spirit/include/qi.hpp>
#include "lexer/SpiritLexer.hpp"
#include "ast/Program.hpp"

#include "parser/ValueGrammar.hpp"

namespace qi = boost::spirit::qi;

namespace eddic {

typedef lexer_type::iterator_type Iterator;
typedef SimpleLexer<lexer_type> Lexer;

struct BooleanGrammar : qi::grammar<Iterator, ast::Condition()> {
    BooleanGrammar(const Lexer& lexer);

    qi::rule<Iterator, ast::Condition()> condition;
    qi::rule<Iterator, ast::True()> true_;
    qi::rule<Iterator, ast::False()> false_;
    qi::rule<Iterator, ast::BinaryCondition()> binary_condition;
    
    ValueGrammar value;
};

} //end of eddic

#endif
