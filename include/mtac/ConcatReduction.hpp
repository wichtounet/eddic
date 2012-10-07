//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONCAT_REDUCTION_H
#define MTAC_CONCAT_REDUCTION_H

#include <memory>

#include "mtac/Function.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

class StringPool;

namespace mtac {

struct optimize_concat {
    std::shared_ptr<StringPool> pool;

    void set_pool(std::shared_ptr<StringPool> pool);

    bool operator()(std::shared_ptr<mtac::Function> function);
};

template<>
struct pass_traits<optimize_concat> {
    STATIC_CONSTANT(pass_type, type, pass_type::CUSTOM);
    STATIC_STRING(name, "optimize_concat");
    STATIC_CONSTANT(bool, need_pool, true);
    STATIC_CONSTANT(bool, need_platform, false);
};

} //end of mtac

} //end of eddic

#endif
