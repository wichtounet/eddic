//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTEL_X86_64_CODE_GENERATOR_H
#define INTEL_X86_64_CODE_GENERATOR_H

#include "asm/IntelCodeGenerator.hpp"

namespace eddic {

namespace as {

class IntelX86_64CodeGenerator : public IntelCodeGenerator {
    public:
        IntelX86_64CodeGenerator(AssemblyFileWriter& writer);
        
        void writeRuntimeSupport(FunctionTable& table);
        void addStandardFunctions();
};

} //end of as

} //end of eddic

#endif

