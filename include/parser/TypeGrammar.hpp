//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef TYPE_GRAMMAR_H
#define TYPE_GRAMMAR_H

#include "boost_cfg.hpp"
#include <boost/spirit/include/qi.hpp>

#include "lexer/SpiritLexer.hpp"

#include "ast/VariableType.hpp"

namespace qi = boost::spirit::qi;

namespace eddic {

namespace parser {

/*!
 * \class TypeGrammar
 * \brief Grammar representing types in EDDI language.
 */
struct TypeGrammar : qi::grammar<lexer::StaticIterator, ast::Type()> {
    TypeGrammar(const lexer::StaticLexer& lexer);

    qi::rule<lexer::StaticIterator, ast::ArrayType()> array_type;
    qi::rule<lexer::StaticIterator, ast::PointerType()> pointer_type;
    qi::rule<lexer::StaticIterator, ast::TemplateType()> template_type;
    qi::rule<lexer::StaticIterator, ast::Type()> type;
};

} //end of parser

} //end of eddic

#endif
