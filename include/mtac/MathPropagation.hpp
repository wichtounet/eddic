//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_MATH_PROPAGATION_H
#define TAC_MATH_PROPAGATION_H

#include <memory>
#include <unordered_map>

#include <boost/variant/static_visitor.hpp>

#include "mtac/Quadruple.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/If.hpp"

#include "mtac/Optimizer.hpp"
#include "mtac/OptimizerUtils.hpp"

namespace eddic {

namespace tac {

class MathPropagation : public boost::static_visitor<void> {
    public:
        bool optimized;
        Pass pass;

        MathPropagation() : optimized(false) {}

        void operator()(std::shared_ptr<tac::Quadruple>& quadruple);
        void operator()(std::shared_ptr<tac::IfFalse>& ifFalse);
        void operator()(std::shared_ptr<tac::If>& if_);

        template<typename T>
        void operator()(T&) const { 
            //Nothing to optimize here
        }
    
        void collect(tac::Argument* arg);
        void collect(boost::optional<tac::Argument>& arg);

    private:
        std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<tac::Quadruple>> assigns;
        std::unordered_map<std::shared_ptr<Variable>, int> usage;
};


} //end of tac

} //end of eddic

#endif
