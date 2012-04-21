//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_COMPILER_H
#define TAC_COMPILER_H

#include "ast/source_def.hpp"

namespace eddic {

class StringPool;

namespace mtac {

class Program;

struct Compiler {
    void compile(ast::SourceFile& program, StringPool& pool, mtac::Program& mtacProgram) const ;
};

} //end of mtac

} //end of eddic

#endif
