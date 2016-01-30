//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_PARAMETER_PROPAGATION_H
#define MTAC_PARAMETER_PROPAGATION_H

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct parameter_propagation {
    bool operator()(mtac::Program& program);
};

template<>
struct pass_traits<parameter_propagation> {
    STATIC_CONSTANT(pass_type, type, pass_type::IPA);
    STATIC_STRING(name, "parameter_propagation");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
