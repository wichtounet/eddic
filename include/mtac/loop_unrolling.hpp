//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_LOOP_UNROLLING_H
#define MTAC_LOOP_UNROLLING_H

#include <memory>

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct loop_unrolling {
    mtac::Program& program;

    loop_unrolling(mtac::Program& program) : program(program){}

    bool gate(std::shared_ptr<Configuration> configuration);
    bool operator()(mtac::Function& function);
};

template<>
struct pass_traits<loop_unrolling> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "loop_unrolling");
    STATIC_CONSTANT(unsigned int, property_flags, PROPERTY_PROGRAM);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
