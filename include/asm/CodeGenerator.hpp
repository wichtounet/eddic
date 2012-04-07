//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "tac/Program.hpp"

#include <memory>

namespace eddic {

class SymbolTable;
class StringPool;
class AssemblyFileWriter;

namespace as {

class CodeGenerator {
    public:
        CodeGenerator(AssemblyFileWriter& writer);
        
        //Entry point
        virtual void generate(tac::Program& program, StringPool& pool, SymbolTable& table) = 0;

    protected:
        AssemblyFileWriter& writer;
};

} //end of as

} //end of eddic

#endif
