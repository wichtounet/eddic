//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cctype>
#include <list>
#include <string>

#include "Types.hpp"
#include "Utils.hpp"

#include "Parser.hpp"
#include "Operators.hpp"
#include "Lexer.hpp"
#include "Branches.hpp"

using std::string;
using std::ios_base;
using std::list;

using namespace eddic;

inline static void assertNextIsRightParenth(Lexer& lexer, const string& message);
inline static void assertNextIsLeftParenth(Lexer& lexer, const string& message);
inline static void assertNextIsRightBrace(Lexer& lexer, const string& message);
inline static void assertNextIsLeftBrace(Lexer& lexer, const string& message);
inline static void assertNextIsStop(Lexer& lexer, const string& message);

//Declare in header
bool isType(const Lexer& lexer){
    if(!lexer.isWord()){
        return false;
    }

    string value = lexer.getCurrentToken().value();

    return value == "int" || value == "string";
}

Program* Parser::parse() {
    Program* program = new Program();

    while (lexer.next()) {
        program->addLast(parseInstruction());
    }

    return program;
}

ParseNode* Parser::parseInstruction() {
    if (lexer.isIf()) {
        return parseIf();
    } else if(isType(lexer)){
        return parseDeclaration();
    } else if(lexer.isWord()){
        return parseCallOrAssignment();
    }

    throw TokenException("Not an instruction", lexer.getCurrentToken());
}

ParseNode* Parser::parseCallOrAssignment(){
    Token token = lexer.getCurrentToken();
    
    if (!lexer.next()) {
        throw TokenException("Incomplete instruction", lexer.getCurrentToken());
    }
    
    if (lexer.isLeftParenth()) {
        return parseCall(token);
    } else if (lexer.isAssign()) { 
        return parseAssignment(token);
    } else if (lexer.isSwap()) {
        return parseSwap(token);
    } 
    
    throw TokenException("Not an instruction", lexer.getCurrentToken());
}

ParseNode* Parser::parseCall(const Token& callToken) {
    string call = callToken.value();

    if (call != "Print" && call != "Println") {
        throw TokenException("The call \"" + call + "\" does not exist", callToken);
    }

    Value* value = parseValue();

    assertNextIsRightParenth(lexer, "The call must be closed with a right parenth");
    assertNextIsStop(lexer, "Every instruction must be closed by a semicolon");

    if (call == "Print") {
        return new Print(value);
    } else {
        return new Println(value);
    }
}

ParseNode* Parser::parseDeclaration() {
    string typeName = lexer.getCurrentToken().value();
    
    Type type;
    if (typeName == "int") {
        type = INT;
    } else {
        type = STRING;
    }

    if(!lexer.next() || !lexer.isWord()){
        throw TokenException("A type must be followed by variable name", lexer.getCurrentToken());
    }

    string variable = lexer.getCurrentToken().value();

    if (!lexer.next() || !lexer.isAssign()) {
        throw TokenException("A variable declaration must followed by '='", lexer.getCurrentToken());
    }

    Value* value = parseValue();

    assertNextIsStop(lexer, "Every instruction must be closed by a semicolon");

    return new Declaration(type, variable, value);
}

ParseNode* Parser::parseAssignment(const Token& variableToken) {
    Value* value = parseValue();

    assertNextIsStop(lexer, "Every instruction must be closed by a semicolon");

    return new Assignment(variableToken.value(), value);
}

ParseNode* Parser::parseSwap(const Token& lhs) {
    if (!lexer.next() || !lexer.isWord()) {
        throw TokenException("Can only swap two variables", lexer.getCurrentToken());
    }

    string rhs = lexer.getCurrentToken().value();

    assertNextIsStop(lexer, "Every instruction must be closed by a semicolon");

    return new Swap(lhs.value(), rhs);
}

ParseNode* Parser::parseIf() {
    assertNextIsLeftParenth(lexer, "An if instruction must be followed by a condition surrounded by parenth");

    Condition* condition = parseCondition();

    assertNextIsRightParenth(lexer, "The condition of the if must be closed by a right parenth");

    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    If* block = new If(condition);

    lexer.next();

    while (!lexer.isRightBrace()) {
        block->addLast(parseInstruction());

        lexer.next();
    }

    if (!lexer.isRightBrace()) {
        throw TokenException("If body must be closed with right brace", lexer.getCurrentToken());
    }

    while (true) {
        if (lexer.next()) {
            if (lexer.isElse()) {
                lexer.next();

                if (lexer.isIf()) {
                    block->addElseIf(parseElseIf());
                } else {
                    lexer.pushBack();

                    block->setElse(parseElse());

                    break;
                }
            } else {
                lexer.pushBack();

                break;
            }
        } else {
            break;
        }
    }

    return block;
}

ElseIf* Parser::parseElseIf() {
    assertNextIsLeftParenth(lexer, "An else if instruction must be followed by a condition surrounded by parenth");

    Condition* condition = parseCondition();

    assertNextIsRightParenth(lexer, "The condition of the else if must be closed by a right parenth");

    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    ElseIf* block = new ElseIf(condition);

    lexer.next();

    while (!lexer.isRightBrace()) {
        block->addLast(parseInstruction());

        lexer.next();
    }

    if (!lexer.isRightBrace()) {
        throw TokenException("Else ff body must be closed with right brace", lexer.getCurrentToken());
    }

    return block;
}

Else* Parser::parseElse() {
    Else* block = new Else();

    assertNextIsLeftBrace(lexer, "else statement must be followed by left brace");

    while (lexer.next() && !lexer.isRightBrace()) {
        block->addLast(parseInstruction());
    }

    if (!lexer.isRightBrace()) {
        throw TokenException("else body must be closed with right brace", lexer.getCurrentToken());
    }

    return block;
}

enum Operator {
    ADD, MUL, SUB, DIV, MOD, ERROR
};

class Part {
    public:
        virtual bool isResolved() = 0;
        virtual Operator getOperator() {
            return ERROR;
        }
        virtual Value* getValue() {
            return NULL;
        }
};

class Resolved : public Part {
    public:
        explicit Resolved(Value* v) : value(v) {}
        bool isResolved() {
            return true;
        }
        Value* getValue() {
            return value;
        }

    private:
        Value* value;
};

class Unresolved : public Part {
    public:
        explicit Unresolved(Operator o) : op(o) {}
        bool isResolved() {
            return false;
        }
        Operator getOperator() {
            return op;
        }

    private:
        Operator op;
};

int priority(Operator op) {
    switch (op) {
        case MUL:
        case DIV:
        case MOD:
            return 10;
        case ADD:
        case SUB:
            return 0;
        default:
            return -1; //TODO should never happen
    }
}

Value* Parser::parseValue() {
    list<Part*> parts;

    while (true) {
        if (!lexer.next()) {
            throw TokenException("Waiting for a value", lexer.getCurrentToken());
        }

        Value* node = NULL;

        if (lexer.isLitteral()) {
            string litteral = lexer.getCurrentToken().value();

            node = new Litteral(litteral);
        } else if (lexer.isWord()) {
            string variableRight = lexer.getCurrentToken().value();

            node = new VariableValue(variableRight);
        } else if (lexer.isInteger()) {
            string integer = lexer.getCurrentToken().value();
            int value = toNumber<int>(integer);

            node = new Integer(value);
        } else {
            throw TokenException("Invalid value", lexer.getCurrentToken());
        }

        parts.push_back(new Resolved(node));

        if (!lexer.next()) {
            break;
        }

        if (lexer.isAddition()) {
            parts.push_back(new Unresolved(ADD));
        } else if (lexer.isSubtraction()) {
            parts.push_back(new Unresolved(SUB));
        } else if (lexer.isMultiplication()) {
            parts.push_back(new Unresolved(MUL));
        } else if (lexer.isDivision()) {
            parts.push_back(new Unresolved(DIV));
        } else if (lexer.isModulo()) {
            parts.push_back(new Unresolved(MOD));
        } else {
            lexer.pushBack();
            break;
        }
    }

    while (parts.size() > 1) {
        int maxPriority = -1;
        list<Part*>::iterator it, end, max;
        for (it = parts.begin(), end = parts.end(); it != end; ++it) {
            Part* part = *it;

            if (!part->isResolved()) {
                if (priority(part->getOperator()) > maxPriority) {
                    maxPriority = priority(part->getOperator());
                    max = it;
                }
            }
        }

        list<Part*>::iterator first = max;
        --first;

        Part* left = *first;
        Part* center = *max;

        ++max;
        Part* right = *max;

        Value* lhs = left->getValue();
        Operator op = center->getOperator();
        Value* rhs = right->getValue();

        Value* value = NULL;

        if (op == ADD) {
            value = new Addition(lhs, rhs);
        } else if (op == SUB) {
            value = new Subtraction(lhs, rhs);
        } else if (op == MUL) {
            value = new Multiplication(lhs, rhs);
        } else if (op == DIV) {
            value = new Division(lhs, rhs);
        } else if (op == MOD) {
            value = new Modulo(lhs, rhs);
        }

        parts.erase(first, ++max);
        parts.insert(max, new Resolved(value));
    }

    Value* value = (*parts.begin())->getValue();

    parts.clear();

    return value;
}

Condition* Parser::parseCondition() {
    lexer.next();

    if (lexer.isTrue()) {
        return new Condition(TRUE_VALUE);
    } else if (lexer.isFalse()) {
        return new Condition(FALSE_VALUE);
    } else {
        lexer.pushBack();
    }

    Value* lhs = parseValue();

    if (!lexer.next()) {
        throw TokenException("waiting for a boolean operator", lexer.getCurrentToken());
    }

    BooleanCondition operation;
    if (lexer.isGreater()) {
        operation = GREATER_OPERATOR;
    } else if (lexer.isLess()) {
        operation = LESS_OPERATOR;
    } else if (lexer.isEquals()) {
        operation = EQUALS_OPERATOR;
    } else if (lexer.isNotEquals()) {
        operation = NOT_EQUALS_OPERATOR;
    } else if (lexer.isGreaterOrEquals()) {
        operation = GREATER_EQUALS_OPERATOR;
    } else if (lexer.isLessOrEquals()) {
        operation = LESS_EQUALS_OPERATOR;
    } else {
        throw TokenException("waiting for a boolean operator", lexer.getCurrentToken());
    }

    Value* rhs = parseValue();

    return new Condition(operation, lhs, rhs);
}

inline static void assertNextIsRightParenth(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isRightParenth()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

inline static void assertNextIsLeftParenth(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isLeftParenth()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

inline static void assertNextIsRightBrace(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isRightBrace()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

inline static void assertNextIsLeftBrace(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isLeftBrace()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

inline static void assertNextIsStop(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isStop()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}
