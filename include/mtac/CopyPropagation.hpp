//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_COPY_PROPAGATION_H
#define TAC_COPY_PROPAGATION_H

#include <memory>

#include <boost/variant/static_visitor.hpp>

#include "mtac/Quadruple.hpp"
#include "mtac/If.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/Param.hpp"
#include "mtac/OptimizerUtils.hpp"

namespace eddic {

namespace mtac {

class CopyPropagation : public boost::static_visitor<void> {
    public:
        bool optimized;

        CopyPropagation() : optimized(false) {}

        void operator()(std::shared_ptr<mtac::Quadruple>& quadruple);
        void operator()(std::shared_ptr<mtac::Param>& param);
        void operator()(std::shared_ptr<mtac::IfFalse>& ifFalse);
        void operator()(std::shared_ptr<mtac::If>& if_);

        template<typename T>
        void operator()(T&) const { 
            //Nothing to optimize
        }

    private:
        std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> constants;

        void optimize(mtac::Argument* arg);
        void optimize_optional(boost::optional<mtac::Argument>& arg);
};

class OffsetCopyPropagation : public boost::static_visitor<void> {
    public:
        bool optimized;

        OffsetCopyPropagation() : optimized(false) {}

        void operator()(std::shared_ptr<mtac::Quadruple>& quadruple);

        template<typename T>
        void operator()(T&){ 
            //Nothing to optimize here
        }

    private:
        std::unordered_map<mtac::Offset, std::shared_ptr<Variable>, mtac::OffsetHash> constants;
};

} //end of tac

} //end of eddic

#endif
