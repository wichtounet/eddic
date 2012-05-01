//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "ltac/Program.hpp"

#include <memory>

namespace eddic {

class StringPool;
class AssemblyFileWriter;

namespace as {

class CodeGenerator {
    public:
        CodeGenerator(AssemblyFileWriter& writer);
        
        //Entry point
        virtual void generate(ltac::Program& program, StringPool& pool) = 0;

    protected:
        AssemblyFileWriter& writer;
};

} //end of as

} //end of eddic

#endif
