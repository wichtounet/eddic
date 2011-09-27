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
    Token* old = read.top();

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

        current = new Token(value, LITTERAL, line, col);

        return true;
    } else if (scanner.isAlpha()) {
        string value = string(1, scanner.current());

        while (scanner.next() && scanner.isAlpha()) {
            value += scanner.current();
        }

        scanner.pushBack();

        if (value == "true") {
            current = new Token(TRUE_TYPE, line, col);
        } else if (value == "false") {
            current = new Token(FALSE_TYPE, line, col);
        } else if (value == "if") {
            current = new Token(IF, line, col);
        } else if (value == "else") {
            current = new Token(ELSE, line, col);
        } else if (value == "while") {
            current = new Token(WHILE, line, col);
        } else if (value == "for") {
            current = new Token(FOR, line, col);
        } else {
            current = new Token(value, WORD, line, col);
        }

        return true;
    } else if (scanner.isDigit()) {
        string value = string(1, scanner.current());

        while (scanner.next() && scanner.isDigit()) {
            value += scanner.current();
        }

        scanner.pushBack();

        current = new Token(value, INTEGER, line, col);

        return true;
    }

    switch (scanner.current()) {
        case ';':
            current = new Token(STOP, line, col);
            return true;
        case ',':
            current = new Token(COMMA, line, col);
            return true;
        case '=':
            scanner.next();

            if (scanner.current() == '=') {
                current = new Token(EQUALS_TOKEN, line, col);
            } else {
                scanner.pushBack();

                current = new Token(ASSIGN, line, col);
            }

            return true;
        case '(':
            current = new Token(LEFT_PARENTH, line, col);
            return true;
        case ')':
            current = new Token(RIGHT_PARENTH, line, col);
            return true;
        case '{':
            current = new Token(LEFT_BRACE, line, col);
            return true;
        case '}':
            current = new Token(RIGHT_BRACE, line, col);
            return true;
        case '+':
            current = new Token(ADDITION, line, col);
            return true;
        case '-':
            current = new Token(SUBTRACTION, line, col);
            return true;
        case '*':
            current = new Token(MULTIPLICATION, line, col);
            return true;
        case '/':
            scanner.next();

            //TODO Find a better way to handle comments
            if(scanner.current() == '/'){
                while(scanner.next() && scanner.current() != '\n'){}

                return next();
            } else if (scanner.current() == '*'){
                while(scanner.next()){
                    if(scanner.current() == '*'){
                        if(scanner.next() && scanner.current() == '/'){
                            break;
                        }
                    }
                }
              
                return next();
            } else {
                scanner.pushBack();
                
                current = new Token(DIVISION, line, col);
            }
            
            return true;
        case '%':
            current = new Token(MODULO, line, col);
            return true;
        case '!':
            scanner.next();

            if (scanner.current() == '=') {
                current = new Token(NOT_EQUALS_TOKEN, line, col);

                return true;
            }

            return false;
        case '<':
            scanner.next();

            if (scanner.current() == '=') {
                scanner.next();

                if (scanner.current() == '>') {
                    current = new Token(SWAP, line, col);
                } else {
                    scanner.pushBack();
                    current = new Token(LESS_EQUALS_TOKEN, line, col);
                }
            } else {
                scanner.pushBack();

                current = new Token(LESS_TOKEN, line, col);
            }

            return true;
        case '>':
            scanner.next();

            if (scanner.current() == '=') {
                current = new Token(GREATER_EQUALS_TOKEN, line, col);
            } else {
                scanner.pushBack();

                current = new Token(GREATER_TOKEN, line, col);
            }

            return true;
    }

    return false;
}

Token* Lexer::getCurrentToken() const {
    return current;
}

bool Lexer::isWord() const {
    return current->type() == WORD;
}

bool Lexer::isLitteral() const {
    return current->type() == LITTERAL;
}

bool Lexer::isAssign() const {
    return current->type() == ASSIGN;
}

bool Lexer::isSwap() const {
    return current->type() == SWAP;
}

bool Lexer::isLeftParenth() const {
    return current->type() == LEFT_PARENTH;
}

bool Lexer::isRightParenth() const {
    return current->type() == RIGHT_PARENTH;
}

bool Lexer::isLeftBrace() const {
    return current->type() == LEFT_BRACE;
}

bool Lexer::isRightBrace() const {
    return current->type() == RIGHT_BRACE;
}

bool Lexer::isStop() const {
    return current->type() == STOP;
}

bool Lexer::isInteger() const {
    return current->type() == INTEGER;
}

bool Lexer::isAddition() const {
    return current->type() == ADDITION;
}

bool Lexer::isSubtraction() const {
    return current->type() == SUBTRACTION;
}

bool Lexer::isMultiplication() const {
    return current->type() == MULTIPLICATION;
}

bool Lexer::isModulo() const {
    return current->type() == MODULO;
}

bool Lexer::isDivision() const {
    return current->type() == DIVISION;
}

bool Lexer::isEquals() const {
    return current->type() == EQUALS_TOKEN;
}

bool Lexer::isNotEquals() const {
    return current->type() == NOT_EQUALS_TOKEN;
}

bool Lexer::isGreater() const {
    return current->type() == GREATER_TOKEN;
}

bool Lexer::isLess() const {
    return current->type() == LESS_TOKEN;
}

bool Lexer::isGreaterOrEquals() const {
    return current->type() == GREATER_EQUALS_TOKEN;
}

bool Lexer::isLessOrEquals() const {
    return current->type() == LESS_EQUALS_TOKEN;
}

bool Lexer::isIf() const {
    return current->type() == IF;
}

bool Lexer::isElse() const {
    return current->type() == ELSE;
}

bool Lexer::isWhile() const {
    return current->type() == WHILE;
}

bool Lexer::isFor() const {
    return current->type() == FOR;
}

bool Lexer::isBooleanOperator() const {
    return current->type() >= EQUALS_TOKEN && current->type() <= LESS_EQUALS_TOKEN;
}

bool Lexer::isBoolean() const {
    return current->type() == TRUE_TYPE || current->type() == FALSE_TYPE;
}

bool Lexer::isTrue() const {
    return current->type() == TRUE_TYPE;
}

bool Lexer::isFalse() const {
    return current->type() == FALSE_TYPE;
}

bool Lexer::isComma() const {
    return current->type() == COMMA;
}
