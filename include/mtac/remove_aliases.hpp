//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_REMOVE_ALIASES_H
#define MTAC_REMOVE_ALIASES_H

#include <memory>

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct remove_aliases {
    bool operator()(mtac::Function& function);
};

template<>
struct pass_traits<remove_aliases> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "remove_aliases");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
