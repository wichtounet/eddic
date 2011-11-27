//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PRINT_INTEGER_FUNCTION_H
#define PRINT_INTEGER_FUNCTION_H

#include "il/Instruction.hpp"

namespace eddic {

struct PrintIntegerFunction : public Instruction {
    void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
