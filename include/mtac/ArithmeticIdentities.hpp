//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_ARITHMETIC_IDENTITIES_H
#define MTAC_ARITHMETIC_IDENTIES_H

#include <memory>

#include <boost/variant/static_visitor.hpp>

#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct ArithmeticIdentities : public boost::static_visitor<void> {
    bool optimized;

    ArithmeticIdentities() : optimized(false) {}

    void operator()(std::shared_ptr<mtac::Quadruple>& quadruple);

    template<typename T>
    void operator()(T&) const { 
        //Nothing to optimize
    }
};


} //end of mtac

} //end of eddic

#endif
