//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_REDUCE_IN_STRENGTH_H
#define MTAC_REDUCE_IN_STRENGTH_H

#include "mtac/forward.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

struct ReduceInStrength {
    bool optimized = false;

    void operator()(mtac::Quadruple& quadruple);
};

template<>
struct pass_traits<ReduceInStrength> {
    STATIC_CONSTANT(pass_type, type, pass_type::LOCAL);
    STATIC_STRING(name, "strength_reduction");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
