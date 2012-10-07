//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_DEAD_CODE_ELIMINATION_H
#define MTAC_DEAD_CODE_ELIMINATION_H

#include <memory>

#include "mtac/Function.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

struct dead_code_elimination {
    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<dead_code_elimination> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "dead_code_elimination");
    STATIC_CONSTANT(bool, need_pool, false);
    STATIC_CONSTANT(bool, need_platform, false);
    STATIC_CONSTANT(bool, need_configuration, false);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
