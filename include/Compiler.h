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

#include "Variables.h"
#include "CompilerException.h"
#include "Lexer.h"
#include "ByteCodeFileWriter.h"

using namespace std;

class Compiler {
  public:
    int compile (string file);
  private:
    void compile () throw (CompilerException);
    ByteCodeFileWriter writer;
    Lexer lexer;
    
    void parseCall(string call, Variables& variables) throw (CompilerException);
    void parseAssign(string variable, Variables& variables) throw (CompilerException);
};

#endif
