//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTEL_X86_CODE_GENERATOR_H
#define INTEL_X86_CODE_GENERATOR_H

#include "asm/CodeGenerator.hpp"

namespace eddic {

namespace as {

class IntelX86CodeGenerator : public CodeGenerator {
    public:
        IntelX86CodeGenerator(AssemblyFileWriter& writer);
};

} //end of as

} //end of eddic

#endif
