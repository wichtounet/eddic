//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_COMPILER_H
#define LTAC_COMPILER_H

#include <memory>

#include "ltac/Program.hpp"
#include "mtac/Program.hpp"

namespace eddic {

namespace ltac {

struct Compiler {
    void compile(std::shared_ptr<mtac::Program> source, std::shared_ptr<ltac::Program> target) const ;
    void compile(std::shared_ptr<mtac::Function> src_function, std::shared_ptr<ltac::Function> target_function) const;
};

} //end of ltac

} //end of eddic

#endif
