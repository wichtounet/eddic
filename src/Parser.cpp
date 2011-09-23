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
#include "Loops.hpp"
#include "Functions.hpp"

using std::string;
using std::ios_base;
using std::list;

using namespace eddic;

inline static void assertNextIsRightParenth(Lexer& lexer, const string& message);
inline static void assertNextIsLeftParenth(Lexer& lexer, const string& message);
inline static void assertNextIsRightBrace(Lexer& lexer, const string& message);
inline static void assertNextIsLeftBrace(Lexer& lexer, const string& message);
inline static void assertNextIsStop(Lexer& lexer, const string& message);

//Move to some utility class
bool isType(const Lexer& lexer) {
    if (!lexer.isWord()) {
        return false;
    }

    string value = lexer.getCurrentToken()->value();

    return value == "int" || value == "string";
}

Program* Parser::parse() {
    //Create the global context
    currentContext = new Context();

    Program* program = new Program(currentContext);

    while (lexer.next()) {
        program->addFunction(parseFunction());
    }

    return program;
}

Function* Parser::parseFunction() {
    if(!lexer.isWord()){
        throw TokenException("Not a function", lexer.getCurrentToken());
    }

    string returnType = lexer.getCurrentToken()->value();

    if(returnType != "void"){
        throw TokenException("Invalid return type", lexer.getCurrentToken());
    }

    lexer.next();

    if(!lexer.isWord()){
        throw TokenException("Expecting a function name", lexer.getCurrentToken());
    }

    string functionName = lexer.getCurrentToken()->value();

    currentContext = new Context(currentContext);

    Function* function = new Function(currentContext, functionName);

    assertNextIsLeftParenth(lexer, "Waiting for a left parenth");
    assertNextIsRightParenth(lexer, "Waiting for a right parenth");

    assertNextIsLeftBrace(lexer, "The instructions of the function must be enclosed in braces");

    lexer.next();

    while (!lexer.isRightBrace()) {
        function->addLast(parseInstruction());

        lexer.next();
    }

    currentContext = currentContext->parent();

    if (!lexer.isRightBrace()) {
        throw TokenException("If body must be closed with right brace", lexer.getCurrentToken());
    }

    return function;
}

ParseNode* Parser::parseInstruction() {
    ParseNode* instruction = NULL;
    
    if (lexer.isIf()) {
        return parseIf();
    } else if (lexer.isWhile()) {
        return parseWhile();
    } else if (lexer.isFor()) {
        return parseFor();
    } else if (isType(lexer)) {
        instruction = parseDeclaration();
    } else if (lexer.isWord()) {
        instruction = parseCallOrAssignment();
    }

    if(instruction == NULL){
        throw TokenException("Not an instruction", lexer.getCurrentToken());
    }
    
    assertNextIsStop(lexer, "Every instruction must be closed by a semicolon");
    
    return instruction;
}

ParseNode* Parser::parseRepeatableInstruction() {
    if (lexer.isWord()) {
        return parseCallOrAssignment();
    }

    throw TokenException("Waiting for a repeatable instruction", lexer.getCurrentToken());
}

ParseNode* Parser::parseCallOrAssignment() {
    Token* token = lexer.getCurrentToken();

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

ParseNode* Parser::parseCall(const Token* callToken) {
    string call = callToken->value();

    if (call != "Print" && call != "Println") {
        FunctionCall* functionCall = new FunctionCall(currentContext, lexer.getCurrentToken(), call);

        assertNextIsRightParenth(lexer, "The function call must be closed with a right parenth");
        
        return functionCall;
    }

    Value* value = parseValue();

    assertNextIsRightParenth(lexer, "The call must be closed with a right parenth");

    if (call == "Print") {
        return new Print(currentContext, value);
    } else {
        return new Println(currentContext, value);
    }
}

ParseNode* Parser::parseDeclaration() {
    string typeName = lexer.getCurrentToken()->value();

    Type type;
    if (typeName == "int") {
        type = INT;
    } else {
        type = STRING;
    }

    if (!lexer.next() || !lexer.isWord()) {
        throw TokenException("A type must be followed by variable name", lexer.getCurrentToken());
    }

    string variable = lexer.getCurrentToken()->value();

    if (!lexer.next() || !lexer.isAssign()) {
        throw TokenException("A variable declaration must followed by '='", lexer.getCurrentToken());
    }

    Value* value = parseValue();

    return new Declaration(currentContext, type, variable, value);
}

ParseNode* Parser::parseAssignment(const Token* variableToken) {
    Value* value = parseValue();

    return new Assignment(currentContext, variableToken->value(), value);
}

ParseNode* Parser::parseSwap(const Token* lhs) {
    if (!lexer.next() || !lexer.isWord()) {
        throw TokenException("Can only swap two variables", lexer.getCurrentToken());
    }

    string rhs = lexer.getCurrentToken()->value();

    return new Swap(currentContext, lhs->value(), rhs);
}

ParseNode* Parser::parseIf() {
    assertNextIsLeftParenth(lexer, "An if instruction must be followed by a condition surrounded by parenth");

    Condition* condition = parseCondition();

    assertNextIsRightParenth(lexer, "The condition of the if must be closed by a right parenth");

    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    currentContext = new Context(currentContext);

    If* block = new If(currentContext, condition);

    lexer.next();

    while (!lexer.isRightBrace()) {
        block->addLast(parseInstruction());

        lexer.next();
    }

    currentContext = currentContext->parent();

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

    currentContext = new Context(currentContext);

    ElseIf* block = new ElseIf(currentContext, condition);

    lexer.next();

    while (!lexer.isRightBrace()) {
        block->addLast(parseInstruction());

        lexer.next();
    }

    currentContext = currentContext->parent();

    if (!lexer.isRightBrace()) {
        throw TokenException("Else ff body must be closed with right brace", lexer.getCurrentToken());
    }

    return block;
}

Else* Parser::parseElse() {
    currentContext = new Context(currentContext);

    Else* block = new Else(currentContext);

    assertNextIsLeftBrace(lexer, "else statement must be followed by left brace");

    while (lexer.next() && !lexer.isRightBrace()) {
        block->addLast(parseInstruction());
    }

    if (!lexer.isRightBrace()) {
        throw TokenException("else body must be closed with right brace", lexer.getCurrentToken());
    }

    currentContext = currentContext->parent();

    return block;
}

ParseNode* Parser::parseWhile() {
    assertNextIsLeftParenth(lexer, "A while instruction must be followed by a condition surrounded by parenth");

    Condition* condition = parseCondition();

    assertNextIsRightParenth(lexer, "The condition of the while must be closed by a right parenth");

    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    currentContext = new Context(currentContext);

    While* block = new While(currentContext, condition);

    lexer.next();

    while (!lexer.isRightBrace()) {
        block->addLast(parseInstruction());

        lexer.next();
    }

    currentContext = currentContext->parent();

    if (!lexer.isRightBrace()) {
        throw TokenException("If body must be closed with right brace", lexer.getCurrentToken());
    }

    return block;
}

ParseNode* Parser::parseFor() {
    assertNextIsLeftParenth(lexer, "A for loop declaration must be followed by a left parenth");

    currentContext = new Context(currentContext);

    lexer.next();

    //TODO Test for type

    ParseNode* start = parseDeclaration();

    assertNextIsStop(lexer, "The start instruction of the for loop must be closed by a semicolon");

    Condition* condition = parseCondition();

    assertNextIsStop(lexer, "The condition of the for loop must be closed by a semicolon");

    lexer.next();

    ParseNode* iter = parseRepeatableInstruction();
    
    assertNextIsRightParenth(lexer, "The components of the for loop must be closed by a right parenth");
    
    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    For* block = new For(currentContext, start, condition, iter);

    lexer.next();

    while (!lexer.isRightBrace()) {
        block->addLast(parseInstruction());

        lexer.next();
    }

    currentContext = currentContext->parent();

    if (!lexer.isRightBrace()) {
        throw TokenException("If body must be closed with right brace", lexer.getCurrentToken());
    }

    return block;
}

enum Operator {
    ADD, MUL, SUB, DIV, MOD, ERROR
};

class Part {
    private:
        Value* value;
        Operator op;

    public:
        explicit Part(Value* v) : value(v), op(ERROR) {}
        explicit Part(Operator o) : value(NULL), op(o) {}
        
        bool isResolved(){
            return value != NULL;
        }

        Operator getOperator() {
            return op;
        }
        
        Value* getValue() {
            return value;
        }
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

        if (lexer.isLeftParenth()) {
            node = parseValue();

            assertNextIsRightParenth(lexer, "parenth is not closed");
        } else if (lexer.isLitteral()) {
            string litteral = lexer.getCurrentToken()->value();

            node = new Litteral(currentContext, litteral);
        } else if (lexer.isWord()) {
            string variableRight = lexer.getCurrentToken()->value();

            node = new VariableValue(currentContext, variableRight);
        } else if (lexer.isInteger()) {
            string integer = lexer.getCurrentToken()->value();
            int value = toNumber<int>(integer);

            node = new Integer(currentContext, value);
        } else {
            throw TokenException("Invalid value", lexer.getCurrentToken());
        }

        parts.push_back(new Part(node));

        if (!lexer.next()) {
            break;
        }

        if (lexer.isAddition()) {
            parts.push_back(new Part(ADD));
        } else if (lexer.isSubtraction()) {
            parts.push_back(new Part(SUB));
        } else if (lexer.isMultiplication()) {
            parts.push_back(new Part(MUL));
        } else if (lexer.isDivision()) {
            parts.push_back(new Part(DIV));
        } else if (lexer.isModulo()) {
            parts.push_back(new Part(MOD));
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
            value = new Addition(currentContext, lhs, rhs);
        } else if (op == SUB) {
            value = new Subtraction(currentContext, lhs, rhs);
        } else if (op == MUL) {
            value = new Multiplication(currentContext, lhs, rhs);
        } else if (op == DIV) {
            value = new Division(currentContext, lhs, rhs);
        } else if (op == MOD) {
            value = new Modulo(currentContext, lhs, rhs);
        }

        parts.erase(first, ++max);

        delete left;
        delete center;
        delete right;

        parts.insert(max, new Part(value));
    }

    Value* value = (*parts.begin())->getValue();

    delete *parts.begin();

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
