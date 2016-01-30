//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_REMOVE_DEAD_BLOCKS_H
#define MTAC_REMOVE_DEAD_BLOCKS_H

#include <memory>

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct remove_dead_basic_blocks {
    mtac::Program& program;

    remove_dead_basic_blocks(mtac::Program& program) : program(program){}

    bool operator()(mtac::Function& function);
};

template<>
struct pass_traits<remove_dead_basic_blocks> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "remove_dead_bb");
    STATIC_CONSTANT(unsigned int, property_flags, PROPERTY_PROGRAM);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
