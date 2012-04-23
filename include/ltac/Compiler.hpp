//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_COMPILER_H
#define LTAC_COMPILER_H

#include "ltac/Program.hpp"
#include "mtac/Program.hpp"

namespace eddic {

namespace ltac {

struct Compiler {
    void compile(std::shared_ptr<mtac::Program> source, std::shared_ptr<ltac::Program> target) const ;
};

} //end of ltac

} //end of eddic

#endif
