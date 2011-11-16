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
#include "ast/Program.hpp"

#include "parser/ValueGrammar.hpp"

namespace qi = boost::spirit::qi;

namespace eddic {

typedef lexer_type::iterator_type Iterator;
typedef SimpleLexer<lexer_type> Lexer;

struct EddiGrammar : qi::grammar<Iterator, ast::Program()> {
    EddiGrammar(const Lexer& lexer);

    qi::rule<Iterator, ast::Type()> type;
    qi::rule<Iterator, ast::ArrayType()> arrayType;
    qi::rule<Iterator, ast::SimpleType()> simpleType;

    qi::rule<Iterator, ast::Program()> program;
    qi::rule<Iterator, ast::GlobalVariableDeclaration()> globalDeclaration;
    qi::rule<Iterator, ast::GlobalArrayDeclaration()> globalArrayDeclaration;
    qi::rule<Iterator, ast::FunctionDeclaration()> function;
    qi::rule<Iterator, ast::FunctionParameter()> arg;

    qi::rule<Iterator, ast::Instruction()> instruction;
    qi::rule<Iterator, ast::Instruction()> repeatable_instruction;
    qi::rule<Iterator, ast::Swap()> swap;
    qi::rule<Iterator, ast::FunctionCall()> functionCall;
    qi::rule<Iterator, ast::VariableDeclaration()> declaration;
    qi::rule<Iterator, ast::ArrayDeclaration()> arrayDeclaration;
    qi::rule<Iterator, ast::Assignment()> assignment;
    qi::rule<Iterator, ast::ArrayAssignment()> arrayAssignment;
    qi::rule<Iterator, ast::While()> while_;
    qi::rule<Iterator, ast::For()> for_;
    qi::rule<Iterator, ast::Foreach()> foreach_;
    qi::rule<Iterator, ast::If()> if_;

    qi::rule<Iterator, ast::Else()> else_;
    qi::rule<Iterator, ast::ElseIf()> else_if_;

    qi::rule<Iterator, ast::Condition()> condition;
    qi::rule<Iterator, ast::True()> true_;
    qi::rule<Iterator, ast::False()> false_;
    qi::rule<Iterator, ast::BinaryCondition()> binary_condition;

    ValueGrammar value;
};

} //end of eddic

#endif
