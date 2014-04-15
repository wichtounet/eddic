//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_PURE_ANALYSIS_H
#define MTAC_PURE_ANALYSIS_H

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct pure_analysis {
    bool operator()(mtac::Program& program);
};

template<>
struct pass_traits<pure_analysis> {
    STATIC_CONSTANT(pass_type, type, pass_type::IPA);
    STATIC_STRING(name, "pure_analysis");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
