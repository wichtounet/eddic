//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONSTANT_FOLDING_H
#define MTAC_CONSTANT_FOLDING_H

#include <memory>

#include "variant.hpp"

#include "mtac/forward.hpp"
#include "mtac/pass_traits.hpp"

namespace eddic {

namespace mtac {

struct ConstantFolding : public boost::static_visitor<void> {
    bool optimized = false;

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple);
    void operator()(std::shared_ptr<mtac::IfFalse> ifFalse);
    void operator()(std::shared_ptr<mtac::If> if_);

    template<typename T>
    void operator()(T&) const { 
        //Nothing to optimize
    }
};

template<>
struct pass_traits<ConstantFolding> {
    STATIC_CONSTANT(pass_type, type, pass_type::LOCAL);
    STATIC_STRING(name, "constant_folding");
    STATIC_CONSTANT(unsigned int, property_flags, 0);
    STATIC_CONSTANT(unsigned int, todo_after_flags, 0);
};

} //end of mtac

} //end of eddic

#endif
