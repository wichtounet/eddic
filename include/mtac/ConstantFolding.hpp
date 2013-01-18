//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONSTANT_FOLDING_H
#define MTAC_CONSTANT_FOLDING_H

#include "mtac/forward.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

struct ConstantFolding {
    bool optimized = false;

    void operator()(mtac::Quadruple& quadruple);
};

template<>
struct pass_traits<ConstantFolding> {
    STATIC_CONSTANT(pass_type, type, pass_type::LOCAL);
    STATIC_STRING(name, "constant_folding");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
