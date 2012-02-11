//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTEL_CODE_GENERATOR_H
#define INTEL_CODE_GENERATOR_H

#include <string>

#include "tac/Program.hpp"

#include "Platform.hpp"

namespace eddic {

class AssemblyFileWriter;
class Context;
class StringPool;
class FunctionTable;

namespace as {

struct StatementCompiler;

class IntelCodeGenerator {
    public:
        IntelCodeGenerator(AssemblyFileWriter& writer);
        
        void generate(tac::Program& program, StringPool& pool, FunctionTable& table, Platform platform);

    private:
        AssemblyFileWriter& writer;
};

} //end of as

} //end of eddic

#endif
