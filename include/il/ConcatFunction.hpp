//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONCAT_FUNCTION_H
#define CONCAT_FUNCTION_H

#include "il/Instruction.hpp"

namespace eddic {

struct ConcatFunction : public Instruction {
    void write(AssemblyFileWriter& writer) const ;
};

} //end of eddic

#endif
