//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "CompilerException.hpp"

namespace eddic {

class Lexer;
class Program;
class ParseNode;
class ElseIf;
class Else;
class Value;
class Condition;

class Parser {
    private:
        Lexer& lexer;

        ParseNode* parseInstruction();
        ParseNode* parseCall(const std::string& call);
        ParseNode* parseDeclaration(const std::string& type);
        ParseNode* parseAssignment(const std::string& variable);
        ParseNode* parseSwap(const std::string& lhs);
        ParseNode* parseIf();
        Condition* parseCondition();
        ElseIf* parseElseIf();
        Else* parseElse();
        Value* parseValue();

    public:
        Parser(Lexer& l) : lexer(l) {};
        Program* parse() ;
};

} //end of eddic

#endif
