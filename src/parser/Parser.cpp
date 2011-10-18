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

#include "parser/Parser.hpp"

#include "Program.hpp"
#include "Integer.hpp"
#include "Litteral.hpp"
#include "Declaration.hpp"
#include "GlobalDeclaration.hpp"
#include "Assignment.hpp"
#include "Context.hpp"
#include "BlockContext.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "Print.hpp"
#include "Println.hpp"
#include "Swap.hpp"
#include "VariableValue.hpp"

#include "BinaryOperator.hpp"
#include "Addition.hpp"
#include "Division.hpp"
#include "Multiplication.hpp"
#include "Subtraction.hpp"
#include "Modulo.hpp"

#include "BooleanCondition.hpp"
#include "Condition.hpp"
#include "Else.hpp"
#include "ElseIf.hpp"
#include "If.hpp"

#include "For.hpp"
#include "Foreach.hpp"
#include "While.hpp"

#include "Function.hpp"
#include "Parameter.hpp"
#include "FunctionCall.hpp"

#include "TokenException.hpp"

using std::string;
using std::ios_base;
using std::list;

using namespace eddic;

template<typename T>
T get(const Tok& token){
    assert(false); //The type is not implemented
}

template<>
std::string get(const Tok& token){
    //std::string tokenvalue(token.value().begin(), token.value().end());
    //std::cout << "Parser::value() = " << tokenvalue << std::endl;
    return "tokenvalue";
}

template<>
int get(const Tok& token){
    int result;
    std::stringstream ss(get<std::string>(token));
    ss >> result;
    
    return result;
}

//Move to some utility class
bool isTokenType(const SpiritLexer& lexer) {
    if (!lexer.isWord()) {
        return false;
    }

    string value = get<std::string>(lexer.getCurrentToken());
	
    return isType(value);
}

const SpiritLexer& Parser::getLexer(){
	return lexer;
}

std::shared_ptr<Program> Parser::parse(const std::string& file) {
	lexer.lex(file);

    //Create the global context
    globalContext = std::shared_ptr<GlobalContext>(new GlobalContext());
    currentContext = globalContext;

    std::shared_ptr<Program> program(new Program(currentContext, lexer.getDefaultToken()));

    /*while (lexer.next()) {
        if(!isTokenType(lexer)){
            throw TokenException("A function or a global variable must start with a type", lexer.getCurrentToken()); 
        }
		
        Type type = stringToType(get<std::string>(lexer.getCurrentToken()));

        assertNextIsWord("A function or a global variable must have a name");

        string name = get<std::string>(lexer.getCurrentToken());

        lexer.next();

        if(lexer.isLeftParenth()){
            program->addFunction(parseFunction(type, name));
        } else if(lexer.isAssign()){
            program->addLast(parseGlobalDeclaration(type, name));
        } else {
            throw TokenException("The body of the file can only contains global variables and function declarations", lexer.getCurrentToken());
        }
    }*/

    return program;
}

std::shared_ptr<Function> Parser::parseFunction(Type type, const string& functionName) {
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

            string typeName = get<std::string>(lexer.getCurrentToken());

            Type type = stringToType(typeName);

            assertNextIsWord("Expecting a parameter name");

            string parameterName = get<std::string>(lexer.getCurrentToken());

            if(params.find(parameterName) != params.end()){
                throw TokenException("The parameter's name must be unique", lexer.getCurrentToken());
            }

            params.insert(parameterName);

            function->addParameter(parameterName, type);
            functionContext->addParameter(parameterName, type);
        }
    }

    assertNextIsLeftBrace("The instructions of the function must be enclosed in braces");

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

std::shared_ptr<ParseNode> Parser::parseGlobalDeclaration(Type type, const string& variable){
    if(type == Type::VOID){
        throw TokenException("void is not a valid type for a variable", lexer.getCurrentToken());
    }

    auto value = parseValue();

    assertNextIsStop("A variable declaration must be followed by a ;");

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
    
    assertNextIsStop("Every instruction must be closed by a semicolon");
    
    return instruction;
}

std::shared_ptr<ParseNode> Parser::parseRepeatableInstruction() {
    if (lexer.isWord()) {
        return parseCallOrAssignment();
    }

    throw TokenException("Waiting for a repeatable instruction", lexer.getCurrentToken());
}

std::shared_ptr<ParseNode> Parser::parseCallOrAssignment() {
    Tok token = lexer.getCurrentToken();

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

std::shared_ptr<ParseNode> Parser::parseCall(const Tok callToken) {
    string call = get<std::string>(callToken);

    if (call != "print" && call != "println") {
        std::shared_ptr<FunctionCall> functionCall(new FunctionCall(currentContext, lexer.getCurrentToken(), call));

        lexer.next();

        if(!lexer.isRightParenth()){
            lexer.pushBack(lexer.getCurrentToken());

            while(!lexer.isRightParenth()){
                auto value = parseValue();

                functionCall->addValue(value);

                lexer.next();
            }
        }
        
        return functionCall;
    }

    auto value = parseValue();

    assertNextIsRightParenth("The call must be closed with a right parenth");

    if (call == "print") {
        return std::shared_ptr<ParseNode>(new Print(currentContext, callToken, value));
    } else {
        return std::shared_ptr<ParseNode>(new Println(currentContext, callToken, value));
    }
}

std::shared_ptr<ParseNode> Parser::parseDeclaration() {
    string typeName = get<std::string>(lexer.getCurrentToken());

    Type type = stringToType(typeName);

    assertNextIsWord("A type must be followed by variable name"); 

    string variable = get<std::string>(lexer.getCurrentToken());

    if (!lexer.next() || !lexer.isAssign()) {
        throw TokenException("A variable declaration must followed by '='", lexer.getCurrentToken());
    }

    auto value = parseValue();

    return std::shared_ptr<ParseNode>(new Declaration(currentContext, lexer.getCurrentToken(), type, variable, value));
}

std::shared_ptr<ParseNode> Parser::parseAssignment(const Tok variableToken) {
    auto value = parseValue();

    return std::shared_ptr<ParseNode>(new Assignment(currentContext, variableToken, get<std::string>(variableToken), value));
}

std::shared_ptr<ParseNode> Parser::parseSwap(const Tok lhs) {
    if (!lexer.next() || !lexer.isWord()) {
        throw TokenException("Can only swap two variables", lexer.getCurrentToken());
    }

    string rhs = get<std::string>(lexer.getCurrentToken());

    return std::shared_ptr<ParseNode>(new Swap(currentContext, lexer.getCurrentToken(), get<std::string>(lhs), rhs));
}

std::shared_ptr<ParseNode> Parser::parseIf() {
    assertNextIsLeftParenth("An if instruction must be followed by a condition surrounded by parenth");

    auto condition = parseCondition();

    assertNextIsRightParenth("The condition of the if must be closed by a right parenth");

    assertNextIsLeftBrace("Waiting for a left brace");

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
                    lexer.pushBack(lexer.getCurrentToken());

                    block->setElse(parseElse());

                    break;
                }
            } else {
                lexer.pushBack(lexer.getCurrentToken());

                break;
            }
        } else {
            break;
        }
    }

    return block;
}

std::shared_ptr<ElseIf> Parser::parseElseIf() {
    assertNextIsLeftParenth("An else if instruction must be followed by a condition surrounded by parenth");

    auto condition = parseCondition();

    assertNextIsRightParenth("The condition of the else if must be closed by a right parenth");

    assertNextIsLeftBrace("Waiting for a left brace");

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

    assertNextIsLeftBrace("else statement must be followed by left brace");

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
    assertNextIsLeftParenth("A while instruction must be followed by a condition surrounded by parenth");

    auto token = lexer.getCurrentToken();

    auto condition = parseCondition();

    assertNextIsRightParenth("The condition of the while must be closed by a right parenth");

    assertNextIsLeftBrace("Waiting for a left brace");

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
    assertNextIsLeftParenth("A for loop declaration must be followed by a left parenth");

    currentContext = std::shared_ptr<Context>(new BlockContext(currentContext, functionContext));
    
    auto token = lexer.getCurrentToken();

    lexer.next();

    //TODO Test for type

    std::shared_ptr<ParseNode> start = parseDeclaration();

    assertNextIsStop("The start instruction of the for loop must be closed by a semicolon");

    auto condition = parseCondition();

    assertNextIsStop("The condition of the for loop must be closed by a semicolon");

    lexer.next();

    auto iter = parseRepeatableInstruction();
    
    assertNextIsRightParenth("The components of the for loop must be closed by a right parenth");
    
    assertNextIsLeftBrace("Waiting for a left brace");

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
    assertNextIsLeftParenth("A foreach loop declaration must be followed by a left parenth");

    currentContext = std::shared_ptr<Context>(new BlockContext(currentContext, functionContext));

    auto token = lexer.getCurrentToken();

    lexer.next();

    if(!isTokenType(lexer)){
        throw TokenException("The foreach must be followed by a type", lexer.getCurrentToken());
    }

    string typeName = get<std::string>(lexer.getCurrentToken());

    Type type = stringToType(typeName);

    if(type != Type::INT){
        throw TokenException("The foreach variable type must be int", lexer.getCurrentToken());
    }

    assertNextIsWord("The type must be followed by a variable name");

    string variable = get<std::string>(lexer.getCurrentToken());

    if(!lexer.next() || !lexer.isFrom()){
        throw TokenException("The foreach variable must be followed by the from declaration", lexer.getCurrentToken());
    }

    auto fromValue = parseValue();

    if(!lexer.next() || !lexer.isTo()){
        throw TokenException("The foreach variable must be followed by the from declaration", lexer.getCurrentToken());
    }

    auto toValue = parseValue();

    assertNextIsRightParenth("The components of the for loop must be closed by a right parenth");

    assertNextIsLeftBrace("Waiting for a left brace");

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

            assertNextIsRightParenth("parenth is not closed");
        } else if (lexer.isLitteral()) {
            string litteral = get<std::string>(lexer.getCurrentToken());

            node = std::shared_ptr<Value>(new Litteral(currentContext, lexer.getCurrentToken(), litteral));
        } else if (lexer.isWord()) {
            string variableRight = get<std::string>(lexer.getCurrentToken());

            node = std::shared_ptr<Value>(new VariableValue(currentContext, lexer.getCurrentToken(), variableRight));
        } else if (lexer.isInteger()) {
            int value = get<int>(lexer.getCurrentToken());

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
            lexer.pushBack(lexer.getCurrentToken());
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
        lexer.pushBack(lexer.getCurrentToken());
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

void Parser::assertNextIsRightParenth(const string& message) {
    if (!lexer.next() || !lexer.isRightParenth()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

void Parser::assertNextIsLeftParenth(const string& message) {
    if (!lexer.next() || !lexer.isLeftParenth()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

void Parser::assertNextIsRightBrace(const string& message) {
    if (!lexer.next() || !lexer.isRightBrace()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

void Parser::assertNextIsLeftBrace(const string& message) {
    if (!lexer.next() || !lexer.isLeftBrace()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

void Parser::assertNextIsStop(const string& message) {
    if (!lexer.next() || !lexer.isStop()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}

void Parser::assertNextIsWord(const string& message) {
    if (!lexer.next() || !lexer.isWord()) {
        throw TokenException(message, lexer.getCurrentToken());
    }
}
