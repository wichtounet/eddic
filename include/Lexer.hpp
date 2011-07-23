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

#include "Scanner.hpp"
#include "CompilerException.hpp"
#include "Token.hpp"

namespace eddic {

class Lexer {
    private:
        Scanner scanner;

        Token current;
        
        std::stack<Token> read;
        std::stack<Token> buffer;

        bool readNext();
    public:
        Lexer() : current(Token(NOTHING, -1, -1)) {};
 
        void lex(std::string file);
        void close();
        bool next();
        void pushBack();
        
        Token getCurrentToken() const;

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
        bool isWhile() const;
        bool isBooleanOperator() const;
        bool isBoolean() const;
        bool isTrue() const;
        bool isFalse() const;
};

} //end of eddic

#endif
