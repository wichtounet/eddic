//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include <string>

#include "ast/Program.hpp"

namespace eddic {

class StringPool;
class FunctionTable;
class IntermediateProgram;

void defineContexts(ASTProgram& program);

void checkVariables(ASTProgram& program);
void checkStrings(ASTProgram& program, StringPool& pool);
void checkFunctions(ASTProgram& program, FunctionTable& functionTable);
void optimize(ASTProgram& program);
void writeIL(ASTProgram& program, StringPool& pool, IntermediateProgram& intermediateProgram);
void writeAsm(IntermediateProgram& il, const std::string& file);

void execCommand(const std::string& command);

struct Compiler {
    int compile (const std::string& file);
};

} //end of eddic

#endif
