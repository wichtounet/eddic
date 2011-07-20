//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Lexer.hpp"

using std::pair;
using std::string;
using std::ios_base;

using namespace eddic;

void Lexer::lex(string file) {
	scanner.scan(file);

    currentType = NOTHING;
}

void Lexer::close() {
    scanner.close();
}

bool Lexer::next() {
    if(!buffer.empty()) {
        pair<string, TokenType> old = buffer.top();

        currentToken = old.first;
        currentType = old.second;

        buffer.pop();

        return true;
    } else if(readNext()) {
        read.push(make_pair(currentToken, currentType));

        return true;
    }

    return false;
}

void Lexer::pushBack() {
    pair<string, TokenType> old = read.top();

    buffer.push(old);

    read.pop();
}

bool Lexer::readNext() {
    if(!scanner.next()) {
        return false;
    }

    while(scanner.isSpace()) { 
		if(!scanner.next()){
			return false;
		}
	}

    if(scanner.current() == '"') {
        currentToken = string(1, scanner.current());

        while(scanner.next() && scanner.current() != '"') {
            currentToken += scanner.current();
        }

        currentToken += scanner.current();

        currentType = LITTERAL;

        return true;
    } else if(scanner.isAlpha()) {
        currentToken = string(1, scanner.current());

        while(scanner.next() && scanner.isAlpha()) {
            currentToken += scanner.current();
        }

        scanner.pushBack();

        if(currentToken == "true") {
            currentType = TRUE;
        } else if(currentToken == "false") {
            currentType = FALSE;
        } else if(currentToken == "if") {
            currentType = IF;
        } else if(currentToken == "else") {
            currentType = ELSE;
        } else {
            currentType = WORD;
        }

        return true;
    } else if(scanner.isDigit()) {
        currentToken = string(1, scanner.current());

        while(scanner.next() && scanner.isDigit()) {
            currentToken += scanner.current();
        }

        scanner.pushBack();

        currentType = INTEGER;

        return true;
    }

    switch (scanner.current()) {
        case ';':
            currentType = STOP;
            return true;
        case '=':
            scanner.next();

            if(scanner.current() == '=') {
                currentType = EQUALS_TOKEN;
            } else {
                scanner.pushBack();

                currentType = ASSIGN;
            }

            return true;
        case '(':
            currentType = LEFT_PARENTH;
            return true;
        case ')':
            currentType = RIGHT_PARENTH;
            return true;
        case '{':
            currentType = LEFT_BRACE;
            return true;
        case '}':
            currentType = RIGHT_BRACE;
            return true;
        case '+':
            currentType = ADDITION;
            return true;
        case '-':
            currentType = SUBTRACTION;
            return true;
        case '*':
            currentType = MULTIPLICATION;
            return true;
        case '/':
            currentType = DIVISION;
            return true;
        case '%':
            currentType = MODULO;
            return true;
        case '!':
            scanner.next();

            if(scanner.current() == '=') {
                currentType = NOT_EQUALS_TOKEN;
            } 

            return false;
        case '<':
            scanner.next();

            if(scanner.current() == '=') {
                scanner.next();

                if(scanner.current() == '>'){
                    currentType = SWAP;
                } else {
                    scanner.pushBack();
                    currentType = LESS_EQUALS_TOKEN;
                }
            } else {
                scanner.pushBack();

                currentType = LESS_TOKEN;
            }

            return true;
        case '>':
            scanner.next();

            if(scanner.current() == '=') {
                currentType = GREATER_EQUALS_TOKEN;
            } else {
                scanner.pushBack();

                currentType = GREATER_TOKEN;
            }

            return true;
    }

    return false;
}

string Lexer::getCurrentToken() const {
    return currentToken;
}

bool Lexer::isWord() const {
    return currentType == WORD;
}

bool Lexer::isLitteral() const {
    return currentType == LITTERAL;
}

bool Lexer::isAssign() const {
    return currentType == ASSIGN;
}

bool Lexer::isSwap() const {
    return currentType == SWAP;
}

bool Lexer::isLeftParenth() const {
    return currentType == LEFT_PARENTH;
}

bool Lexer::isRightParenth() const {
    return currentType == RIGHT_PARENTH;
}

bool Lexer::isLeftBrace() const {
    return currentType == LEFT_BRACE;
}

bool Lexer::isRightBrace() const {
    return currentType == RIGHT_BRACE;
}

bool Lexer::isStop() const {
    return currentType == STOP;
}

bool Lexer::isInteger() const {
    return currentType == INTEGER;
}

bool Lexer::isAddition() const {
    return currentType == ADDITION;
}

bool Lexer::isSubtraction() const {
    return currentType == SUBTRACTION;
}

bool Lexer::isMultiplication() const {
    return currentType == MULTIPLICATION;
}

bool Lexer::isModulo() const {
    return currentType == MODULO;
}

bool Lexer::isDivision() const {
    return currentType == DIVISION;
}

bool Lexer::isEquals() const {
    return currentType == EQUALS_TOKEN;
}

bool Lexer::isNotEquals() const {
    return currentType == NOT_EQUALS_TOKEN;
}

bool Lexer::isGreater() const {
    return currentType == GREATER_TOKEN;
}

bool Lexer::isLess() const {
    return currentType == LESS_TOKEN;
}

bool Lexer::isGreaterOrEquals() const {
    return currentType == GREATER_EQUALS_TOKEN;
}

bool Lexer::isLessOrEquals() const {
    return currentType == LESS_EQUALS_TOKEN;
}

bool Lexer::isIf() const {
    return currentType == IF;
}

bool Lexer::isElse() const {
    return currentType == ELSE;
}

bool Lexer::isBooleanOperator() const {
    return currentType >= EQUALS_TOKEN && currentType <= LESS_EQUALS_TOKEN;
}

bool Lexer::isBoolean() const {
    return currentType == TRUE || currentType == FALSE;
}

bool Lexer::isTrue() const {
    return currentType == TRUE;
}

bool Lexer::isFalse() const {
    return currentType == FALSE;
}
