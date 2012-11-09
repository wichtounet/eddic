//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LOOP_ANALYSIS_H
#define MTAC_LOOP_ANALYSIS_H

#include "mtac/pass_traits.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

void full_loop_analysis(mtac::program_p program);
void full_loop_analysis(mtac::function_p function);

struct loop_analysis {
    bool operator()(mtac::function_p function);
};

template<>
struct pass_traits<loop_analysis> {
    STATIC_STRING(name, "loop_analysis");
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
