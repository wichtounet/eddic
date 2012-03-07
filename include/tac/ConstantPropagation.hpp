//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_CONSTANT_PROPAGATION_H
#define TAC_CONSTANT_PROPAGATION_H

#include <memory>

#include <boost/variant/static_visitor.hpp>

#include "tac/Quadruple.hpp"
#include "tac/If.hpp"
#include "tac/IfFalse.hpp"
#include "tac/OptimizerUtils.hpp"

namespace eddic {

namespace tac {

class ConstantPropagation : public boost::static_visitor<void> {
    public:
        bool optimized;

        ConstantPropagation() : optimized(false) {}

        void operator()(std::shared_ptr<tac::Quadruple>& quadruple);
        void operator()(std::shared_ptr<tac::IfFalse>& ifFalse);
        void operator()(std::shared_ptr<tac::If>& if_);

        template<typename T>
        void operator()(T&) const { 
            //Nothing to optimize
        }

    private:
        std::unordered_map<std::shared_ptr<Variable>, int> int_constants;
        std::unordered_map<std::shared_ptr<Variable>, std::string> string_constants;

        void optimize(tac::Argument* arg);
        void optimize_optional(boost::optional<tac::Argument>& arg);
};

class OffsetConstantPropagation : public boost::static_visitor<void> {
    public:
        bool optimized;

        OffsetConstantPropagation() : optimized(false) {}

        void operator()(std::shared_ptr<tac::Quadruple>& quadruple);

        template<typename T>
        void operator()(T&){ 
            //Nothing to optimize here
        }

    private:
        std::unordered_map<tac::Offset, int, tac::OffsetHash> int_constants;
        std::unordered_map<tac::Offset, std::string, tac::OffsetHash> string_constants;
};

} //end of tac

} //end of eddic

#endif