//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_INLINING_H
#define MTAC_INLINING_H

#include "Options.hpp"

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct inline_functions {
    bool gate(std::shared_ptr<Configuration> configuration);
    bool operator()(mtac::Program& program);
};

template<>
struct pass_traits<inline_functions> {
    STATIC_CONSTANT(pass_type, type, pass_type::IPA);
    STATIC_STRING(name, "inline_functions");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
