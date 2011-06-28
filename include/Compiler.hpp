//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include <string>

#include "Variables.hpp"
#include "Lexer.hpp"
#include "ByteCodeFileWriter.hpp"
#include "Nodes.hpp"
#include "StringPool.hpp"

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