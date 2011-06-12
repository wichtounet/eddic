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
    bool isCall() const;
    bool isLitteral() const;
    bool isParenth() const;
    bool isLeftParenth() const;
    bool isRightParenth() const;
    bool isStop() const;
};

#endif
