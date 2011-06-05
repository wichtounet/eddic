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
    void next();
    string getCurrentToken();
    bool hasMoreToken();
    bool isCall();
    bool isLitteral();
    bool isParenth();
    bool isLeftParenth();
    bool isRightParenth();
    bool isStop();
};

#endif
