//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include <string>

#include "Lexer.hpp"
#include "ByteCodeFileWriter.hpp"

class Program;
class Variables;
class StringPool;

class Compiler {
  public:
    int compile (std::string file);
  private:
    Lexer lexer;
    ByteCodeFileWriter writer;
    void compile (Program* program);
	void check(Program* program, Variables& variables);
	void checkStrings(Program* program, StringPool& pool);
};

#endif
