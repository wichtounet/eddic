//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPE_GRAMMAR_H
#define TYPE_GRAMMAR_H

#include "boost_cfg.hpp"
#include <boost/spirit/include/qi.hpp>

#include "lexer/SpiritLexer.hpp"

#include "ast/VariableType.hpp"
#include "ast/Struct.hpp"

namespace qi = boost::spirit::qi;

namespace eddic {

namespace parser {

/*!
 * \class TypeGrammar
 * \brief Grammar representing types in EDDI language.
 */
struct TypeGrammar : qi::grammar<lexer::Iterator, ast::Type()> {
    TypeGrammar(const lexer::Lexer& lexer, const lexer::pos_iterator_type& position_begin);

    qi::rule<lexer::Iterator, ast::Type()> type;
    qi::rule<lexer::Iterator, ast::ArrayType()> array_type;
    qi::rule<lexer::Iterator, ast::SimpleType()> simple_type;
    qi::rule<lexer::Iterator, ast::PointerType()> pointer_type;
    qi::rule<lexer::Iterator, ast::TemplateType()> template_type;

    /* Helper  */
    qi::rule<lexer::Iterator, bool()> const_;
    
    const lexer::pos_iterator_type& position_begin;
};

} //end of parser

} //end of eddic

#endif
