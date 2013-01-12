//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_MERGE_BASIC_BLOCKS_H
#define MTAC_MERGE_BASIC_BLOCKS_H

#include <memory>

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct merge_basic_blocks {
    mtac::Program& program;

    merge_basic_blocks(mtac::Program& program) : program(program){}

    bool operator()(mtac::Function& function);
};

template<>
struct pass_traits<merge_basic_blocks> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "merge_bb");
    STATIC_CONSTANT(unsigned int, property_flags, PROPERTY_PROGRAM);
    STATIC_CONSTANT(unsigned int, todo_after_flags, TODO_REMOVE_NOP);
};

} //end of mtac

} //end of eddic

#endif
