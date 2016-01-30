//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
struct EddiGrammar : qi::grammar<lexer::StaticIterator,
    ast::SourceFile(lexer::pos_iterator_type, int),
    qi::locals<lexer::pos_iterator_type, int> >
{
    EddiGrammar(const lexer::StaticLexer& lexer);

  private:
    template <typename A, typename... Inherited> using Rule = qi::rule<lexer::StaticIterator, A(Inherited...), qi::locals<lexer::pos_iterator_type, int>>;
    Rule<ast::SourceFile, lexer::pos_iterator_type, int> start;

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

    /* Structures */
    Rule<ast::struct_definition>    template_struct;
    Rule<ast::MemberDeclaration> member_declaration;
    Rule<ast::Constructor>       constructor;
    Rule<ast::Destructor>        destructor;

    const ValueGrammar value_grammar;
    const TypeGrammar type_grammar;
};

} //end of parser

} //end of eddic

#endif
