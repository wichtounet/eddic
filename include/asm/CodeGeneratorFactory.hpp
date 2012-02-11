//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CODE_GENERATOR_FACTORY_H
#define CODE_GENERATOR_FACTORY_H

#include "Platform.hpp"

#include "asm/CodeGenerator.hpp"

namespace eddic {

namespace as {

class CodeGeneratorFactory {
    public:
        CodeGenerator get(Platform platform, AssemblyFileWriter& writer);
};

} //end of as

} //end of eddic

#endif
