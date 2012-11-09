//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_PROGRAM_H
#define MTAC_PROGRAM_H

#include <memory>

#include "mtac/Function.hpp"

namespace eddic {

struct GlobalContext;

namespace mtac {

enum class Mode : unsigned int {
    MTAC,
    LTAC
};

struct Program {
    std::shared_ptr<GlobalContext> context;

    std::vector<std::shared_ptr<Function>> functions;

    Mode mode = Mode::MTAC;
};

typedef std::shared_ptr<mtac::Program> program_p;

} //end of mtac

} //end of eddic

#endif
