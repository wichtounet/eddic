//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_CONSTANT_FOLDING_H
#define TAC_CONSTANT_FOLDING_H

#include <memory>

#include <boost/variant/static_visitor.hpp>

#include "mtac/Quadruple.hpp"
#include "mtac/If.hpp"
#include "mtac/IfFalse.hpp"

namespace eddic {

namespace tac {

struct ConstantFolding : public boost::static_visitor<void> {
    bool optimized;

    ConstantFolding() : optimized(false) {}

    void operator()(std::shared_ptr<tac::Quadruple>& quadruple);
    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse);
    void operator()(std::shared_ptr<tac::If>& if_);

    template<typename T>
    void operator()(T&) const { 
        //Nothing to optimize
    }
};


} //end of tac

} //end of eddic

#endif
