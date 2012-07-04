//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_MATH_PROPAGATION_H
#define MTAC_MATH_PROPAGATION_H

#include <memory>
#include <unordered_map>

#include <boost/variant/static_visitor.hpp>

#include "mtac/Pass.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/If.hpp"

#include "mtac/Optimizer.hpp"
#include "mtac/OptimizerUtils.hpp"

namespace eddic {

namespace mtac {

class MathPropagation : public boost::static_visitor<void> {
    public:
        bool optimized;
        Pass pass;

        MathPropagation() : optimized(false) {}

        void operator()(std::shared_ptr<mtac::Quadruple>& quadruple);
        void operator()(std::shared_ptr<mtac::IfFalse>& ifFalse);
        void operator()(std::shared_ptr<mtac::If>& if_);

        template<typename T>
        void operator()(T&) const { 
            //Nothing to optimize here
        }
    
        void collect(mtac::Argument* arg);
        void collect(boost::optional<mtac::Argument>& arg);

    private:
        std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<mtac::Quadruple>> assigns;
        std::unordered_map<std::shared_ptr<Variable>, int> usage;
};


} //end of mtac

} //end of eddic

#endif
