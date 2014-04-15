//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_REMOVE_EMPTY_FUNCTIONS_H
#define MTAC_REMOVE_EMPTY_FUNCTIONS_H

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct remove_empty_functions {
    bool operator()(mtac::Program& program);
};

template<>
struct pass_traits<remove_empty_functions> {
    STATIC_CONSTANT(pass_type, type, pass_type::IPA);
    STATIC_STRING(name, "remove_empty_functions");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
