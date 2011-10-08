//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Token.hpp"

using namespace eddic;

Token::Token(std::string value, TokenType type, int line, int col) : m_value(value), m_type(type), m_line(line), m_col(col) {}
Token::Token(TokenType type, int line, int col) : m_value(""), m_type(type), m_line(line), m_col(col) {}

std::string Token::value() const {
    return m_value;
}

TokenType Token::type() const {
    return m_type;
}

int Token::line() const {
    return m_line;
}

int Token::col() const {
    return m_col;
}
