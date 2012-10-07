//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_MERGE_BASIC_BLOCKS_H
#define MTAC_MERGE_BASIC_BLOCKS_H

#include <memory>

#include "mtac/Function.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

struct merge_basic_blocks {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<merge_basic_blocks> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "remove_dead_bb");
    STATIC_CONSTANT(bool, need_pool, false);
    STATIC_CONSTANT(bool, need_platform, false);
};

struct remove_dead_basic_blocks {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<remove_dead_basic_blocks> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "remove_dead_bb");
    STATIC_CONSTANT(bool, need_pool, false);
    STATIC_CONSTANT(bool, need_platform, false);
};

} //end of mtac

} //end of eddic

#endif
