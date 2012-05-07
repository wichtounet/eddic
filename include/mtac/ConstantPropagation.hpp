//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CONSTANT_PROPAGATION_H
#define MTAC_CONSTANT_PROPAGATION_H

#include <memory>

#include <boost/variant/static_visitor.hpp>

#include "mtac/Quadruple.hpp"
#include "mtac/If.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/Param.hpp"
#include "mtac/OptimizerUtils.hpp"

namespace eddic {

namespace mtac {

class ConstantPropagation : public boost::static_visitor<void> {
    public:
        bool optimized;

        ConstantPropagation() : optimized(false) {}

        void operator()(std::shared_ptr<mtac::Quadruple>& quadruple);
        void operator()(std::shared_ptr<mtac::Param>& param);
        void operator()(std::shared_ptr<mtac::IfFalse>& ifFalse);
        void operator()(std::shared_ptr<mtac::If>& if_);

        template<typename T>
        void operator()(T&) const { 
            //Nothing to optimize
        }

    private:
        std::unordered_map<std::shared_ptr<Variable>, int> int_constants;
        std::unordered_map<std::shared_ptr<Variable>, double> float_constants;
        std::unordered_map<std::shared_ptr<Variable>, std::string> string_constants;

        void optimize(mtac::Argument* arg);
        void optimize_optional(boost::optional<mtac::Argument>& arg);
};

class OffsetConstantPropagation : public boost::static_visitor<void> {
    public:
        bool optimized;

        OffsetConstantPropagation() : optimized(false) {}

        void operator()(std::shared_ptr<mtac::Quadruple>& quadruple);

        template<typename T>
        void operator()(T&){ 
            //Nothing to optimize here
        }

    private:
        std::unordered_map<mtac::Offset, int, mtac::OffsetHash> int_constants;
        std::unordered_map<mtac::Offset, std::string, mtac::OffsetHash> string_constants;
};

} //end of mtac

} //end of eddic

#endif
