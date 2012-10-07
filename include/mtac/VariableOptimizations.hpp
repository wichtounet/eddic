//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_VARIABLE_OPTIMIZATIONS_H
#define MTAC_VARIABLE_OPTIMIZATIONS_H

#include <memory>

#include "mtac/Function.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

struct remove_aliases {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<remove_aliases> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "optimize_branches");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

struct clean_variables {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<clean_variables> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "clean_variables");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
