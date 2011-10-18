//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "Context.hpp"
#include "CompilerException.hpp"
#include "lexer/SpiritLexer.hpp"

namespace eddic {

class Program;
class Function;
class ParseNode;
class ElseIf;
class Else;
class Value;
class Condition;
class Context;
class GlobalContext;
class FunctionContext;

class Parser {
    private:
        SpiritLexer lexer;

        std::shared_ptr<GlobalContext> globalContext;
        std::shared_ptr<FunctionContext> functionContext;
        std::shared_ptr<Context> currentContext;

        std::shared_ptr<Function> parseFunction(Type type, const std::string& functionName);
        std::shared_ptr<ParseNode> parseGlobalDeclaration(Type type, const std::string& functionName);
        std::shared_ptr<ParseNode> parseInstruction();
        std::shared_ptr<ParseNode> parseRepeatableInstruction();
        std::shared_ptr<ParseNode> parseDeclaration();
        std::shared_ptr<ParseNode> parseCallOrAssignment();
        std::shared_ptr<ParseNode> parseCall(const Tok call);
        std::shared_ptr<ParseNode> parseAssignment(const Tok variable);
        std::shared_ptr<ParseNode> parseSwap(const Tok lhs);
        std::shared_ptr<ParseNode> parseIf();
        std::shared_ptr<ParseNode> parseWhile();
        std::shared_ptr<ParseNode> parseFor();
        std::shared_ptr<ParseNode> parseForeach();
        std::shared_ptr<Condition> parseCondition();
        std::shared_ptr<ElseIf> parseElseIf();
        std::shared_ptr<Else> parseElse();
        std::shared_ptr<Value> parseValue();

        void assertNextIsRightParenth(const std::string& message);
        void assertNextIsLeftParenth(const std::string& message);
        void assertNextIsRightBrace(const std::string& message);
        void assertNextIsLeftBrace(const std::string& message);
        void assertNextIsStop(const std::string& message);
        void assertNextIsWord(const std::string& message);

    public:
        std::shared_ptr<Program> parse(const std::string& file);

        const SpiritLexer& getLexer();
};

} //end of eddic

#endif
