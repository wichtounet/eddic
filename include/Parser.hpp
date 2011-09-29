//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARSER_H
#define PARSER_H

#include "Context.hpp"
#include "CompilerException.hpp"

namespace eddic {

class Lexer;
class Program;
class Function;
class ParseNode;
class ElseIf;
class Else;
class Value;
class Condition;

class Parser {
    private:
        Lexer& lexer;

        std::shared_ptr<GlobalContext> globalContext;
        std::shared_ptr<FunctionContext> functionContext;
        std::shared_ptr<Context> currentContext;

        std::shared_ptr<Function> parseFunction();
        std::shared_ptr<ParseNode> parseInstruction();
        std::shared_ptr<ParseNode> parseRepeatableInstruction();
        std::shared_ptr<ParseNode> parseDeclaration();
        std::shared_ptr<ParseNode> parseCallOrAssignment();
        std::shared_ptr<ParseNode> parseCall(const std::shared_ptr<Token> call);
        std::shared_ptr<ParseNode> parseAssignment(const std::shared_ptr<Token> variable);
        std::shared_ptr<ParseNode> parseSwap(const std::shared_ptr<Token> lhs);
        std::shared_ptr<ParseNode> parseIf();
        std::shared_ptr<ParseNode> parseWhile();
        std::shared_ptr<ParseNode> parseFor();
        std::shared_ptr<Condition> parseCondition();
        std::shared_ptr<ElseIf> parseElseIf();
        std::shared_ptr<Else> parseElse();
        std::shared_ptr<Value> parseValue();

    public:
        Parser(Lexer& l) : lexer(l) {};
        std::shared_ptr<Program> parse() ;
};

} //end of eddic

#endif
