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

namespace eddic {

namespace mtac {

struct ConstantFolding : public boost::static_visitor<void> {
    bool optimized = false;

    void operator()(std::shared_ptr<mtac::Quadruple>& quadruple);
    void operator()(std::shared_ptr<mtac::IfFalse>& ifFalse);
    void operator()(std::shared_ptr<mtac::If>& if_);

    template<typename T>
    void operator()(T&) const { 
        //Nothing to optimize
    }
};


} //end of mtac

} //end of eddic

#endif
