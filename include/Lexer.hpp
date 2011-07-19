//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LEXER_H
#define LEXER_H

#include <fstream>
#include <string>
#include <utility>
#include <stack>

#include "CompilerException.hpp"

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

    STOP,
    INTEGER,
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULO,
    TRUE,
    FALSE,

    //Boolean operators
    EQUALS_TOKEN,
    NOT_EQUALS_TOKEN,
    GREATER_TOKEN,
    LESS_TOKEN,
    GREATER_EQUALS_TOKEN,
    LESS_EQUALS_TOKEN,

    //Branches
    IF,
    ELSE
};

class Lexer {
    private:
        std::ifstream stream;
        std::string currentToken;
        TokenType currentType;
        std::stack < std::pair< std::string, TokenType > > read;
        std::stack < std::pair< std::string, TokenType > > buffer;

        bool readNext();
    public:
        void lex(std::string file);
        void close();
        bool next();
        void pushBack();
        std::string getCurrentToken() const;

        bool isWord() const;
        bool isAssign() const;
        bool isSwap() const;
        bool isLitteral() const;
        bool isLeftParenth() const;
        bool isRightParenth() const;
        bool isLeftBrace() const;
        bool isRightBrace() const;
        bool isStop() const;
        bool isInteger() const;
        bool isAddition() const;
        bool isSubtraction() const;
        bool isMultiplication() const;
        bool isDivision() const;
        bool isModulo() const;
        bool isEquals() const;
        bool isNotEquals() const;
        bool isGreater() const;
        bool isLess() const;
        bool isGreaterOrEquals() const;
        bool isLessOrEquals() const;
        bool isIf() const;
        bool isElse() const;
        bool isBooleanOperator() const;
        bool isBoolean() const;
        bool isTrue() const;
        bool isFalse() const;
};

} //end of eddic

#endif
