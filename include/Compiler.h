//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include "Variables.h"
#include "CompilerException.h"
#include "Lexer.h"
#include "ByteCodeFileWriter.h"
#include "Nodes.h"
#include "StringPool.h"

class Compiler {
  public:
    int compile (std::string file);
  private:
    void compile (Program* program);
    ByteCodeFileWriter writer;
    Lexer lexer;
	void check(Program* program, Variables& variables);
	void checkStrings(Program* program, StringPool& pool);
};

#endif
