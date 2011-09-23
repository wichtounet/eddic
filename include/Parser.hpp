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
        Context* currentContext;

        Function* parseFunction();
        ParseNode* parseInstruction();
        ParseNode* parseRepeatableInstruction();
        ParseNode* parseDeclaration();
        ParseNode* parseCallOrAssignment();
        ParseNode* parseCall(const Token* call);
        ParseNode* parseAssignment(const Token* variable);
        ParseNode* parseSwap(const Token* lhs);
        ParseNode* parseIf();
        ParseNode* parseWhile();
        ParseNode* parseFor();
        Condition* parseCondition();
        ElseIf* parseElseIf();
        Else* parseElse();
        Value* parseValue();

    public:
        Parser(Lexer& l) : lexer(l), currentContext(new Context()) {};
        Program* parse() ;
};

} //end of eddic

#endif
