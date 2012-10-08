//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LOOP_OPTIMIZATIONS_H
#define MTAC_LOOP_OPTIMIZATIONS_H

#include <memory>

#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

class Function;

struct loop_invariant_code_motion {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<loop_invariant_code_motion> {
    STATIC_STRING(name, "loop_invariant_motion");
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

struct loop_induction_variables_optimization {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<loop_induction_variables_optimization> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "loop_iv_optimization");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

struct remove_empty_loops {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<remove_empty_loops> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "remove_empty_loops");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

struct complete_loop_peeling {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<complete_loop_peeling> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "complete_loop_peeling");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
