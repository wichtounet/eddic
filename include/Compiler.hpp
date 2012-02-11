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

#include "Platform.hpp"

namespace eddic {

namespace parser {
    class SpiritParser;
}

class StringPool;
class FunctionTable;

void clean(ast::SourceFile& program);
void defineDefaultValues(ast::SourceFile& program);
void defineContexts(ast::SourceFile& program);
void defineVariables(ast::SourceFile& program);
void defineFunctions(ast::SourceFile& program, FunctionTable& table);

void includeDependencies(ast::SourceFile& sourceFile, parser::SpiritParser& parser);

void checkTypes(ast::SourceFile& program);
void checkStrings(ast::SourceFile& program, StringPool& pool);

void checkForMain(FunctionTable& table);
void checkForWarnings(ast::SourceFile& program, FunctionTable& table);

void transform(ast::SourceFile& program);
void optimize(ast::SourceFile& program, FunctionTable& functionTable, StringPool& pool);

/*!
 * \class Compiler
 * \brief The EDDI compiler.
 *
 * This class is used to launch the compilation of a source file. It will then launch each phases of the compilation on this phase
 * and produce either an executable or an assembly file depending on the provided options. 
 */
struct Compiler {
    int compile (const std::string& file);
    int compileOnly (const std::string& file, Platform platform);
};

/*!
 * \brief Produces a warning on the command line. 
 * \param warning The warning message to produce.  
 */
void warn(const std::string& warning);

} //end of eddic

#endif
