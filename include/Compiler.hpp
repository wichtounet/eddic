//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPILER_H
#define COMPILER_H

#include <string>

#include "ast/source_def.hpp"

namespace eddic {

class StringPool;
class FunctionTable;
class SpiritParser;

void defineDefaultValues(ast::SourceFile& program);
void defineContexts(ast::SourceFile& program);
void defineVariables(ast::SourceFile& program);
void defineFunctions(ast::SourceFile& program, FunctionTable& table);

void includeDependencies(ast::SourceFile& sourceFile, SpiritParser& parser);

void checkTypes(ast::SourceFile& program);
void checkStrings(ast::SourceFile& program, StringPool& pool);

void checkForWarnings(ast::SourceFile& program, FunctionTable& table);

void transform(ast::SourceFile& program);
void optimize(ast::SourceFile& program, FunctionTable& functionTable, StringPool& pool);

void execCommand(const std::string& command);

struct Compiler {
    int compile (const std::string& file);
};

//Utility for other classes
void warn(const std::string& warning);

} //end of eddic

#endif
