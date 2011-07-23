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
}

void Lexer::close() {
    scanner.close();
}

bool Lexer::next() {
    if (!buffer.empty()) {
        current = buffer.top();

        buffer.pop();

        return true;
    } else if (readNext()) {
        read.push(current);

        return true;
    }

    return false;
}

void Lexer::pushBack() {
    Token old = read.top();

    buffer.push(old);

    read.pop();
}

bool Lexer::readNext() {
    if (!scanner.next()) {
        return false;
    }

    while (scanner.isSpace()) {
        if (!scanner.next()) {
            return false;
        }
    }

    int line = scanner.line();
    int col = scanner.col();

    if (scanner.current() == '"') {
        string value = string(1, scanner.current());

        while (scanner.next() && scanner.current() != '"') {
            value += scanner.current();
        }

        value += scanner.current();

        current = Token(value, LITTERAL, line, col);

        return true;
    } else if (scanner.isAlpha()) {
        string value = string(1, scanner.current());

        while (scanner.next() && scanner.isAlpha()) {
            value += scanner.current();
        }

        scanner.pushBack();

        if (value == "true") {
            current = Token(TRUE_TYPE, line, col);
        } else if (value == "false") {
            current = Token(FALSE_TYPE, line, col);
        } else if (value == "if") {
            current = Token(IF, line, col);
        } else if (value == "else") {
            current = Token(ELSE, line, col);
        } else if (value == "while") {
            current = Token(WHILE, line, col);
        } else {
            current = Token(value, WORD, line, col);
        }

        return true;
    } else if (scanner.isDigit()) {
        string value = string(1, scanner.current());

        while (scanner.next() && scanner.isDigit()) {
            value += scanner.current();
        }

        scanner.pushBack();

        current = Token(value, INTEGER, line, col);

        return true;
    }

    switch (scanner.current()) {
        case ';':
            current = Token(STOP, line, col);
            return true;
        case '=':
            scanner.next();

            if (scanner.current() == '=') {
                current = Token(EQUALS_TOKEN, line, col);
            } else {
                scanner.pushBack();

                current = Token(ASSIGN, line, col);
            }

            return true;
        case '(':
            current = Token(LEFT_PARENTH, line, col);
            return true;
        case ')':
            current = Token(RIGHT_PARENTH, line, col);
            return true;
        case '{':
            current = Token(LEFT_BRACE, line, col);
            return true;
        case '}':
            current = Token(RIGHT_BRACE, line, col);
            return true;
        case '+':
            current = Token(ADDITION, line, col);
            return true;
        case '-':
            current = Token(SUBTRACTION, line, col);
            return true;
        case '*':
            current = Token(MULTIPLICATION, line, col);
            return true;
        case '/':
            current = Token(DIVISION, line, col);
            return true;
        case '%':
            current = Token(MODULO, line, col);
            return true;
        case '!':
            scanner.next();

            if (scanner.current() == '=') {
                current = Token(NOT_EQUALS_TOKEN, line, col);
                
                return true;
            }

            return false;
        case '<':
            scanner.next();

            if (scanner.current() == '=') {
                scanner.next();

                if (scanner.current() == '>') {
                    current = Token(SWAP, line, col);
                } else {
                    scanner.pushBack();
                    current = Token(LESS_EQUALS_TOKEN, line, col);
                }
            } else {
                scanner.pushBack();

                current = Token(LESS_TOKEN, line, col);
            }

            return true;
        case '>':
            scanner.next();

            if (scanner.current() == '=') {
                current = Token(GREATER_EQUALS_TOKEN, line, col);
            } else {
                scanner.pushBack();

                current = Token(GREATER_TOKEN, line, col);
            }

            return true;
    }

    return false;
}

Token Lexer::getCurrentToken() const {
    return current;
}

bool Lexer::isWord() const {
    return current.type() == WORD;
}

bool Lexer::isLitteral() const {
    return current.type() == LITTERAL;
}

bool Lexer::isAssign() const {
    return current.type() == ASSIGN;
}

bool Lexer::isSwap() const {
    return current.type() == SWAP;
}

bool Lexer::isLeftParenth() const {
    return current.type() == LEFT_PARENTH;
}

bool Lexer::isRightParenth() const {
    return current.type() == RIGHT_PARENTH;
}

bool Lexer::isLeftBrace() const {
    return current.type() == LEFT_BRACE;
}

bool Lexer::isRightBrace() const {
    return current.type() == RIGHT_BRACE;
}

bool Lexer::isStop() const {
    return current.type() == STOP;
}

bool Lexer::isInteger() const {
    return current.type() == INTEGER;
}

bool Lexer::isAddition() const {
    return current.type() == ADDITION;
}

bool Lexer::isSubtraction() const {
    return current.type() == SUBTRACTION;
}

bool Lexer::isMultiplication() const {
    return current.type() == MULTIPLICATION;
}

bool Lexer::isModulo() const {
    return current.type() == MODULO;
}

bool Lexer::isDivision() const {
    return current.type() == DIVISION;
}

bool Lexer::isEquals() const {
    return current.type() == EQUALS_TOKEN;
}

bool Lexer::isNotEquals() const {
    return current.type() == NOT_EQUALS_TOKEN;
}

bool Lexer::isGreater() const {
    return current.type() == GREATER_TOKEN;
}

bool Lexer::isLess() const {
    return current.type() == LESS_TOKEN;
}

bool Lexer::isGreaterOrEquals() const {
    return current.type() == GREATER_EQUALS_TOKEN;
}

bool Lexer::isLessOrEquals() const {
    return current.type() == LESS_EQUALS_TOKEN;
}

bool Lexer::isIf() const {
    return current.type() == IF;
}

bool Lexer::isElse() const {
    return current.type() == ELSE;
}

bool Lexer::isWhile() const {
    return current.type() == WHILE;
}

bool Lexer::isBooleanOperator() const {
    return current.type() >= EQUALS_TOKEN && current.type() <= LESS_EQUALS_TOKEN;
}

bool Lexer::isBoolean() const {
    return current.type() == TRUE_TYPE || current.type() == FALSE_TYPE;
}

bool Lexer::isTrue() const {
    return current.type() == TRUE_TYPE;
}

bool Lexer::isFalse() const {
    return current.type() == FALSE_TYPE;
}
