//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef EDDI_GRAMMAR_H
#define EDDI_GRAMMAR_H

#include "boost_cfg.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

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
struct EddiGrammar : qi::grammar<lexer::Iterator, 
    ast::SourceFile(lexer::pos_iterator_type),
    qi::locals<lexer::pos_iterator_type> >
{
    EddiGrammar(const lexer::Lexer& lexer);

  private:
    template <typename A, typename... Inherited> using Rule = qi::rule<lexer::Iterator, A(Inherited...), qi::locals<lexer::pos_iterator_type> >;
    Rule<ast::SourceFile, lexer::pos_iterator_type> start;

    /* First level blocks */
    Rule<ast::SourceFile>                  program;
    Rule<ast::GlobalVariableDeclaration>   globalDeclaration;
    Rule<ast::GlobalArrayDeclaration>      globalArrayDeclaration;
    Rule<ast::TemplateFunctionDeclaration> template_function;
    Rule<ast::FunctionDeclaration>         function;
    Rule<ast::FunctionParameter>           arg;

    /* Instructions */
    Rule<ast::Instruction>         instruction;
    Rule<ast::Instruction>         repeatable_instruction;
    Rule<ast::Swap>                swap;
    Rule<ast::VariableDeclaration> declaration;
    Rule<ast::StructDeclaration>   struct_declaration;
    Rule<ast::ArrayDeclaration>    arrayDeclaration;
    Rule<ast::Return>              return_;
    Rule<ast::Switch>              switch_;
    Rule<ast::SwitchCase>          switch_case;
    Rule<ast::DefaultCase>         default_case;
    Rule<ast::Delete>              delete_;

    /* Loops */
    Rule<ast::While>     while_;
    Rule<ast::DoWhile>   do_while_;
    Rule<ast::For>       for_;
    Rule<ast::Foreach>   foreach_;
    Rule<ast::ForeachIn> foreachin_;

    /* Branches  */
    Rule<ast::If>     if_;
    Rule<ast::Else>   else_;
    Rule<ast::ElseIf> else_if_;
 
    /* Imports  */
    Rule<ast::StandardImport> standardImport;
    Rule<ast::Import>         import;

    /* Structures */
    Rule<ast::Struct>            struct_;
    Rule<ast::TemplateStruct>    template_struct;
    Rule<ast::MemberDeclaration> member_declaration;
    Rule<ast::Constructor>       constructor;
    Rule<ast::Destructor>        destructor;
   
    const ValueGrammar value_grammar;
    const TypeGrammar type_grammar;
};

} //end of parser

} //end of eddic

#endif
