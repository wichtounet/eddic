//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cctype>
#include <list>

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

//TODO Review this method
ParseNode* Parser::parseInstruction() {
    if (lexer.isIf()) {
        return parseIf();
    }

    if (!lexer.isWord()) {
        throw CompilerException("An instruction can only start with a call or an assignation");
    }

    string word = lexer.getCurrentToken();

    if (!lexer.next()) {
        throw CompilerException("Incomplete instruction");
    }

    if (lexer.isLeftParenth()) { //is a call
        return parseCall(word);
    } else if (lexer.isWord()) { //is a declaration
        return parseDeclaration(word);
    } else if (lexer.isAssign()) { //is an assign
        return parseAssignment(word);
    } else if (lexer.isSwap()) {
        return parseSwap(word);
    } else {
        throw CompilerException("Not an instruction");
    }
}

Program* Parser::parse() {
    Program* program = new Program();

    while (lexer.next()) {
        program->addLast(parseInstruction());
    }

    return program;
}

inline static void assertNextIsRightParenth(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isRightParenth()) {
        throw CompilerException(message);
    }
}

inline static void assertNextIsLeftParenth(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isLeftParenth()) {
        throw CompilerException(message);
    }
}

inline static void assertNextIsRightBrace(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isRightBrace()) {
        throw CompilerException(message);
    }
}

inline static void assertNextIsLeftBrace(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isLeftBrace()) {
        throw CompilerException(message);
    }
}

inline static void assertNextIsStop(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isStop()) {
        throw CompilerException(message);
    }
}

ParseNode* Parser::parseCall(const string& call) {
    if (call != "Print" && call != "Println") {
        throw CompilerException("The call \"" + call + "\" does not exist");
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

ParseNode* Parser::parseDeclaration(const string& typeName) {
    if (typeName != "int" && typeName != "string") {
        throw CompilerException("Invalid type");
    }

    Type type;
    if (typeName == "int") {
        type = INT;
    } else {
        type = STRING;
    }

    string variable = lexer.getCurrentToken();

    if (!lexer.next() || !lexer.isAssign()) {
        throw CompilerException("A variable declaration must followed by '='");
    }

    Value* value = parseValue();

    assertNextIsStop(lexer, "Every instruction must be closed by a semicolon");

    return new Declaration(type, variable, value);
}

ParseNode* Parser::parseAssignment(const string& variable) {
    Value* value = parseValue();

    assertNextIsStop(lexer, "Every instruction must be closed by a semicolon");

    return new Assignment(variable, value);
}

ParseNode* Parser::parseSwap(const string& lhs) {
    if (!lexer.next() || !lexer.isWord()) {
        throw CompilerException("Can only swap two variables");
    }

    string rhs = lexer.getCurrentToken();

    assertNextIsStop(lexer, "Every instruction must be closed by a semicolon");

    return new Swap(lhs, rhs);
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
        throw CompilerException("If body must be closed with right brace");
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
        throw CompilerException("Else ff body must be closed with right brace");
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
        throw CompilerException("else body must be closed with right brace");
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
            throw CompilerException("Waiting for a value");
        }

        Value* node = NULL;

        if (lexer.isLitteral()) {
            string litteral = lexer.getCurrentToken();

            node = new Litteral(litteral);
        } else if (lexer.isWord()) {
            string variableRight = lexer.getCurrentToken();

            node = new VariableValue(variableRight);
        } else if (lexer.isInteger()) {
            string integer = lexer.getCurrentToken();
            int value = toNumber<int>(integer);

            node = new Integer(value);
        } else {
            throw CompilerException("Invalid value");
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
        throw CompilerException("waiting for a boolean operator");
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
        throw CompilerException("waiting for a boolean operator");
    }

    Value* rhs = parseValue();

    return new Condition(operation, lhs, rhs);
}

