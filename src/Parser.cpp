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
#include <memory>

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
inline static void assertNextIsWord(Lexer& lexer, const string& message);

//Move to some utility class
bool isTokenType(const Lexer& lexer) {
    if (!lexer.isWord()) {
        return false;
    }

    string value = lexer.getCurrentToken()->value();

    return isType(value);
}

std::shared_ptr<Program> Parser::parse() {
    //Create the global context
    globalContext = std::shared_ptr<GlobalContext>(new GlobalContext());
    currentContext = globalContext;

    std::shared_ptr<Program> program(new Program(currentContext));

    while (lexer.next()) {
        if(!isTokenType(lexer)){
            throw TokenException("A function or a global variable must start with a type", lexer.getCurrentToken()); 
        }

        Type type = stringToType(lexer.getCurrentToken()->value());

        assertNextIsWord(lexer, "A function or a global variable must have a name");

        string name = lexer.getCurrentToken()->value();

        lexer.next();

        if(lexer.isLeftParenth()){
            program->addFunction(parseFunction(type, name));
        } else if(lexer.isAssign()){
            program->addLast(parseGlobalDeclaration(type, name));
        } else {
            throw TokenException("The body of the file can only contains global variables and function declarations", lexer.getCurrentToken());
        }
    }

    return program;
}

std::shared_ptr<Function> Parser::parseFunction(Type type, string functionName) {
    if(type != Type::VOID){
        throw TokenException("The return type of a function must be void", lexer.getCurrentToken());
    }

    functionContext = std::shared_ptr<FunctionContext>(new FunctionContext(currentContext));
    currentContext = functionContext;

    std::shared_ptr<Function> function(new Function(currentContext, lexer.getCurrentToken(), functionName));

    std::unordered_set<std::string> params;

    while(true){
        lexer.next();

        if(lexer.isRightParenth()){
            break;
        } else {
            if(lexer.isComma()){
                lexer.next();
            }

            if(!isTokenType(lexer)){
                throw TokenException("Expecting a parameter type", lexer.getCurrentToken());
            }

            string typeName = lexer.getCurrentToken()->value();

            Type type = stringToType(typeName);

            assertNextIsWord(lexer, "Expecting a parameter name");

            string parameterName = lexer.getCurrentToken()->value();

            if(params.find(parameterName) != params.end()){
                throw TokenException("The parameter's name must be unique", lexer.getCurrentToken());
            }

            params.insert(parameterName);

            function->addParameter(parameterName, type);
            functionContext->addParameter(parameterName, type);
        }
    }

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

std::shared_ptr<ParseNode> Parser::parseGlobalDeclaration(Type type, string variable){
    if(type == Type::VOID){
        throw TokenException("void is not a valid type for a variable", lexer.getCurrentToken());
    }

    auto value = parseValue();

    assertNextIsStop(lexer, "A variable declaration must be followed by a ;");

    return std::shared_ptr<ParseNode>(new GlobalDeclaration(globalContext, lexer.getCurrentToken(), type, variable, value));
}

std::shared_ptr<ParseNode> Parser::parseInstruction() {
    std::shared_ptr<ParseNode> instruction;
    
    if (lexer.isIf()) {
        return parseIf();
    } else if (lexer.isWhile()) {
        return parseWhile();
    } else if (lexer.isFor()) {
        return parseFor();
    } else if (lexer.isForeach()) {
        return parseForeach();
    } else if (isTokenType(lexer)) {
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

std::shared_ptr<ParseNode> Parser::parseRepeatableInstruction() {
    if (lexer.isWord()) {
        return parseCallOrAssignment();
    }

    throw TokenException("Waiting for a repeatable instruction", lexer.getCurrentToken());
}

std::shared_ptr<ParseNode> Parser::parseCallOrAssignment() {
    std::shared_ptr<Token> token = lexer.getCurrentToken();

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

std::shared_ptr<ParseNode> Parser::parseCall(const std::shared_ptr<Token> callToken) {
    string call = callToken->value();

    if (call != "Print" && call != "Println") {
        std::shared_ptr<FunctionCall> functionCall(new FunctionCall(currentContext, lexer.getCurrentToken(), call));

        lexer.next();

        if(!lexer.isRightParenth()){
            lexer.pushBack();

            while(!lexer.isRightParenth()){
                auto value = parseValue();

                functionCall->addValue(value);

                lexer.next();
            }
        }
        
        return functionCall;
    }

    auto value = parseValue();

    assertNextIsRightParenth(lexer, "The call must be closed with a right parenth");

    if (call == "Print") {
        return std::shared_ptr<ParseNode>(new Print(currentContext, callToken, value));
    } else {
        return std::shared_ptr<ParseNode>(new Println(currentContext, callToken, value));
    }
}

std::shared_ptr<ParseNode> Parser::parseDeclaration() {
    string typeName = lexer.getCurrentToken()->value();

    Type type = stringToType(typeName);

    assertNextIsWord(lexer, "A type must be followed by variable name"); 

    string variable = lexer.getCurrentToken()->value();

    if (!lexer.next() || !lexer.isAssign()) {
        throw TokenException("A variable declaration must followed by '='", lexer.getCurrentToken());
    }

    auto value = parseValue();

    return std::shared_ptr<ParseNode>(new Declaration(currentContext, lexer.getCurrentToken(), type, variable, value));
}

std::shared_ptr<ParseNode> Parser::parseAssignment(const std::shared_ptr<Token> variableToken) {
    auto value = parseValue();

    return std::shared_ptr<ParseNode>(new Assignment(currentContext, variableToken, variableToken->value(), value));
}

std::shared_ptr<ParseNode> Parser::parseSwap(const std::shared_ptr<Token> lhs) {
    if (!lexer.next() || !lexer.isWord()) {
        throw TokenException("Can only swap two variables", lexer.getCurrentToken());
    }

    string rhs = lexer.getCurrentToken()->value();

    return std::shared_ptr<ParseNode>(new Swap(currentContext, lexer.getCurrentToken(), lhs->value(), rhs));
}

std::shared_ptr<ParseNode> Parser::parseIf() {
    assertNextIsLeftParenth(lexer, "An if instruction must be followed by a condition surrounded by parenth");

    auto condition = parseCondition();

    assertNextIsRightParenth(lexer, "The condition of the if must be closed by a right parenth");

    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    currentContext = std::shared_ptr<Context>(new BlockContext(currentContext, functionContext));

    std::shared_ptr<If> block(new If(currentContext, lexer.getCurrentToken(), condition));

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

std::shared_ptr<ElseIf> Parser::parseElseIf() {
    assertNextIsLeftParenth(lexer, "An else if instruction must be followed by a condition surrounded by parenth");

    auto condition = parseCondition();

    assertNextIsRightParenth(lexer, "The condition of the else if must be closed by a right parenth");

    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    currentContext = std::shared_ptr<Context>(new BlockContext(currentContext, functionContext));

    std::shared_ptr<ElseIf> block(new ElseIf(currentContext, lexer.getCurrentToken(), condition));

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

std::shared_ptr<Else> Parser::parseElse() {
    currentContext = std::shared_ptr<Context>(new BlockContext(currentContext, functionContext));

    std::shared_ptr<Else> block(new Else(currentContext, lexer.getCurrentToken()));

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

std::shared_ptr<ParseNode> Parser::parseWhile() {
    assertNextIsLeftParenth(lexer, "A while instruction must be followed by a condition surrounded by parenth");

    auto token = lexer.getCurrentToken();

    auto condition = parseCondition();

    assertNextIsRightParenth(lexer, "The condition of the while must be closed by a right parenth");

    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    currentContext = std::shared_ptr<Context>(new BlockContext(currentContext, functionContext));

    std::shared_ptr<While> block(new While(currentContext, token, condition));

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

std::shared_ptr<ParseNode> Parser::parseFor() {
    assertNextIsLeftParenth(lexer, "A for loop declaration must be followed by a left parenth");

    currentContext = std::shared_ptr<Context>(new BlockContext(currentContext, functionContext));
    
    auto token = lexer.getCurrentToken();

    lexer.next();

    //TODO Test for type

    std::shared_ptr<ParseNode> start = parseDeclaration();

    assertNextIsStop(lexer, "The start instruction of the for loop must be closed by a semicolon");

    auto condition = parseCondition();

    assertNextIsStop(lexer, "The condition of the for loop must be closed by a semicolon");

    lexer.next();

    auto iter = parseRepeatableInstruction();
    
    assertNextIsRightParenth(lexer, "The components of the for loop must be closed by a right parenth");
    
    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    std::shared_ptr<For> block(new For(currentContext, token, start, condition, iter));

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

std::shared_ptr<ParseNode> Parser::parseForeach() {
    assertNextIsLeftParenth(lexer, "A foreach loop declaration must be followed by a left parenth");

    currentContext = std::shared_ptr<Context>(new BlockContext(currentContext, functionContext));

    auto token = lexer.getCurrentToken();

    lexer.next();

    if(!isTokenType(lexer)){
        throw TokenException("The foreach must be followed by a type", lexer.getCurrentToken());
    }

    string typeName = lexer.getCurrentToken()->value();

    Type type = stringToType(typeName);

    if(type != Type::INT){
        throw TokenException("The foreach variable type must be int", lexer.getCurrentToken());
    }

    assertNextIsWord(lexer, "The type must be followed by a variable name");

    string variable = lexer.getCurrentToken()->value();

    if(!lexer.next() || !lexer.isFrom()){
        throw TokenException("The foreach variable must be followed by the from declaration", lexer.getCurrentToken());
    }

    auto fromValue = parseValue();

    if(!lexer.next() || !lexer.isTo()){
        throw TokenException("The foreach variable must be followed by the from declaration", lexer.getCurrentToken());
    }

    auto toValue = parseValue();

    assertNextIsRightParenth(lexer, "The components of the for loop must be closed by a right parenth");

    assertNextIsLeftBrace(lexer, "Waiting for a left brace");

    std::shared_ptr<Foreach> block(new Foreach(currentContext, token, type, variable, fromValue, toValue));

    lexer.next();

    while (!lexer.isRightBrace()) {
        block->addLast(parseInstruction());

        lexer.next();
    }

    currentContext = currentContext->parent();

    if (!lexer.isRightBrace()) {
        throw TokenException("Foreach body must be closed with right brace", lexer.getCurrentToken());
    }

    return block;
}

enum Operator {
    ADD, MUL, SUB, DIV, MOD, ERROR
};

class Part {
    private:
        std::shared_ptr<Value> value;
        Operator op;

    public:
        explicit Part(std::shared_ptr<Value> v) : value(v), op(ERROR) {}
        explicit Part(Operator o) : value(NULL), op(o) {}
        
        bool isResolved(){
            return value != NULL;
        }

        Operator getOperator() {
            return op;
        }
        
        std::shared_ptr<Value> getValue() {
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

std::shared_ptr<Value> Parser::parseValue() {
    list<Part> parts;

    while (true) {
        if (!lexer.next()) {
            throw TokenException("Waiting for a value", lexer.getCurrentToken());
        }

        std::shared_ptr<Value> node;

        if (lexer.isLeftParenth()) {
            node = parseValue();

            assertNextIsRightParenth(lexer, "parenth is not closed");
        } else if (lexer.isLitteral()) {
            string litteral = lexer.getCurrentToken()->value();

            node = std::shared_ptr<Value>(new Litteral(currentContext, lexer.getCurrentToken(), litteral));
        } else if (lexer.isWord()) {
            string variableRight = lexer.getCurrentToken()->value();

            node = std::shared_ptr<Value>(new VariableValue(currentContext, lexer.getCurrentToken(), variableRight));
        } else if (lexer.isInteger()) {
            string integer = lexer.getCurrentToken()->value();
            int value = toNumber<int>(integer);

            node = std::shared_ptr<Value>(new Integer(currentContext, lexer.getCurrentToken(), value));
        } else {
            throw TokenException("Invalid value", lexer.getCurrentToken());
        }

        parts.push_back(Part(node));

        if (!lexer.next()) {
            break;
        }

        if (lexer.isAddition()) {
            parts.push_back(Part(ADD));
        } else if (lexer.isSubtraction()) {
            parts.push_back(Part(SUB));
        } else if (lexer.isMultiplication()) {
            parts.push_back(Part(MUL));
        } else if (lexer.isDivision()) {
            parts.push_back(Part(DIV));
        } else if (lexer.isModulo()) {
            parts.push_back(Part(MOD));
        } else {
            lexer.pushBack();
            break;
        }
    }

    while (parts.size() > 1) {
        int maxPriority = -1;
        list<Part>::iterator it, end, max;
        for (it = parts.begin(), end = parts.end(); it != end; ++it) {
            Part part = *it;

            if (!part.isResolved()) {
                if (priority(part.getOperator()) > maxPriority) {
                    maxPriority = priority(part.getOperator());
                    max = it;
                }
            }
        }

        auto first = max;
        --first;

        auto left = *first;
        auto center = *max;

        ++max;
        auto right = *max;

        auto lhs = left.getValue();
        auto rhs = right.getValue();
        Operator op = center.getOperator();

        std::shared_ptr<Value> value;

        if (op == ADD) {
            value = std::shared_ptr<Value>(new Addition(currentContext, lhs->token(), lhs, rhs));
        } else if (op == SUB) {
            value = std::shared_ptr<Value>(new Subtraction(currentContext, lhs->token(), lhs, rhs));
        } else if (op == MUL) {
            value = std::shared_ptr<Value>(new Multiplication(currentContext, lhs->token(), lhs, rhs));
        } else if (op == DIV) {
            value = std::shared_ptr<Value>(new Division(currentContext, lhs->token(), lhs, rhs));
        } else if (op == MOD) {
            value = std::shared_ptr<Value>(new Modulo(currentContext, lhs->token(), lhs, rhs));
        }

        parts.erase(first, ++max);

        parts.insert(max, Part(value));
    }

    return (*parts.begin()).getValue();
}

std::shared_ptr<Condition> Parser::parseCondition() {
    lexer.next();

    if (lexer.isTrue()) {
        return std::shared_ptr<Condition>(new Condition(TRUE_VALUE));
    } else if (lexer.isFalse()) {
        return std::shared_ptr<Condition>(new Condition(FALSE_VALUE));
    } else {
        lexer.pushBack();
    }

    auto lhs = parseValue();

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

    auto rhs = parseValue();

    return std::shared_ptr<Condition>(new Condition(operation, lhs, rhs));
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

inline static void assertNextIsWord(Lexer& lexer, const string& message) {
    if (!lexer.next() || !lexer.isWord()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}
