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

extern Platform platform;

class FunctionTable;
void checkForMain(FunctionTable& table);

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

} //end of eddic

#endif
