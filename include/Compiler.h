//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>

#include "Lexer.h"
#include "ByteCodeFileWriter.h"

using namespace std;

class CompilerException: public exception {
  private:
    string message;
  public:
    CompilerException(string m) : message(m) {};
    ~CompilerException() throw() {};
    const char* what() throw();
};

class Compiler {
  public:
    int compile (string file);
  private:
    void compile () throw (CompilerException);
    Lexer lexer;
    ByteCodeFileWriter writer;


    void parseCall(string call, map<string, int>& variables) throw (CompilerException);
    void parseAssign(string variable, map<string, int>& variables, int& currentVariable) throw (CompilerException);
};

#endif
