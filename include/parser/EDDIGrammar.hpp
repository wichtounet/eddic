//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EDDI_GRAMMAR_H
#define EDDI_GRAMMAR_H

#include <boost/spirit/include/qi.hpp>
#include "lexer/SpiritLexer.hpp"
#include "ast/SourceFile.hpp"

#include "parser/ValueGrammar.hpp"
#include "parser/TypeGrammar.hpp"

namespace qi = boost::spirit::qi;

namespace eddic {

namespace parser {

/*!
 * \class EDDIGrammar
 * \brief Grammar representing the whole EDDI syntax.  
 */
struct EddiGrammar : qi::grammar<lexer::Iterator, ast::SourceFile()> {
    EddiGrammar(const lexer::Lexer& lexer, const lexer::pos_iterator_type& position_begin);

    /* First level blocks */
    qi::rule<lexer::Iterator, ast::SourceFile()> program;
    qi::rule<lexer::Iterator, ast::GlobalVariableDeclaration()> globalDeclaration;
    qi::rule<lexer::Iterator, ast::GlobalArrayDeclaration()> globalArrayDeclaration;
    qi::rule<lexer::Iterator, ast::FunctionDeclaration()> function;
    qi::rule<lexer::Iterator, ast::FunctionParameter()> arg;

    /* Instructions */
    qi::rule<lexer::Iterator, ast::Instruction()> instruction;
    qi::rule<lexer::Iterator, ast::Instruction()> repeatable_instruction;
    qi::rule<lexer::Iterator, ast::Swap()> swap;
    qi::rule<lexer::Iterator, ast::VariableDeclaration()> declaration;
    qi::rule<lexer::Iterator, ast::ArrayDeclaration()> arrayDeclaration;
    qi::rule<lexer::Iterator, ast::Return()> return_;
    qi::rule<lexer::Iterator, ast::Switch()> switch_;
    qi::rule<lexer::Iterator, ast::SwitchCase()> switch_case;
    qi::rule<lexer::Iterator, ast::DefaultCase()> default_case;

    /* Loops */
    qi::rule<lexer::Iterator, ast::While()> while_;
    qi::rule<lexer::Iterator, ast::DoWhile()> do_while_;
    qi::rule<lexer::Iterator, ast::For()> for_;
    qi::rule<lexer::Iterator, ast::Foreach()> foreach_;
    qi::rule<lexer::Iterator, ast::ForeachIn()> foreachin_;

    /* Branches  */
    qi::rule<lexer::Iterator, ast::If()> if_;
    qi::rule<lexer::Iterator, ast::Else()> else_;
    qi::rule<lexer::Iterator, ast::ElseIf()> else_if_;
 
    /* Imports  */
    qi::rule<lexer::Iterator, ast::StandardImport()> standardImport;
    qi::rule<lexer::Iterator, ast::Import()> import;

    /* Structures */
    qi::rule<lexer::Iterator, ast::Struct()> struct_;
    qi::rule<lexer::Iterator, ast::MemberDeclaration()> member_declaration;
   
    ValueGrammar value;
    TypeGrammar type;

    const lexer::pos_iterator_type& position_begin;
};

} //end of parser

} //end of eddic

#endif
