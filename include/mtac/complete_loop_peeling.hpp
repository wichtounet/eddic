//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_COMPLETE_LOOP_PEELING_H
#define MTAC_COMPLETE_LOOP_PEELING_H

#include <memory>

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct complete_loop_peeling {
    mtac::Program& program;

    complete_loop_peeling(mtac::Program& program) : program(program){}

    bool gate(std::shared_ptr<Configuration> configuration);
    bool operator()(mtac::Function& function);
};

template<>
struct pass_traits<complete_loop_peeling> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "complete_loop_peeling");
    STATIC_CONSTANT(unsigned int, property_flags, PROPERTY_PROGRAM);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
