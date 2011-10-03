//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TOKEN_H
#define TOKEN_H

#include <string>

namespace eddic {

enum TokenType {
    NOTHING,
    WORD,
    ASSIGN,
    LITTERAL,
    SWAP,

    LEFT_PARENTH,
    RIGHT_PARENTH,
    LEFT_BRACE,
    RIGHT_BRACE,

    COMMA,

    STOP,
    INTEGER,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULO,
    TRUE_TYPE,
    FALSE_TYPE,

    //Boolean operators
    EQUALS_TOKEN,
    NOT_EQUALS_TOKEN,
    GREATER_TOKEN,
    LESS_TOKEN,
    GREATER_EQUALS_TOKEN,
    LESS_EQUALS_TOKEN,

    //Branches
    IF,
    ELSE,
    WHILE,
    FOR,
    FOREACH,
    FROM,
    TO
};

class Token {
    private:
        std::string m_value;
        TokenType m_type;
        int m_line;
        int m_col;

    public:
        Token(std::string value, TokenType type, int line, int col) : m_value(value), m_type(type), m_line(line), m_col(col) {}
        Token(TokenType type, int line, int col) : m_value(""), m_type(type), m_line(line), m_col(col) {}

        std::string value() const {
            return m_value;
        }

        TokenType type() const {
            return m_type;
        }

        int line() const {
            return m_line;
        }

        int col() const {
            return m_col;
        }
};

} //end of eddic

#endif
