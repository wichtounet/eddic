//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

class Lexer {
    ifstream* stream;
    string currentToken;
  public:
    Lexer(ifstream* inStream);
    bool next();
    string getCurrentToken() const;
    bool isWord() const;
    bool isAssign() const;
    bool isLitteral() const;
    bool isParenth() const;
    bool isLeftParenth() const;
    bool isRightParenth() const;
    bool isStop() const;
};

#endif
