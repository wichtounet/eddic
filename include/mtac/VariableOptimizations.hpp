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
    STATIC_CONSTANT(bool, need_pool, false);
    STATIC_CONSTANT(bool, need_platform, false);
};

struct clean_variables {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<clean_variables> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "clean_variables");
    STATIC_CONSTANT(bool, need_pool, false);
    STATIC_CONSTANT(bool, need_platform, false);
};

} //end of mtac

} //end of eddic

#endif
