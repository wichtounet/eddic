//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_REMOVE_MULTIPLE_ASSIGN_H
#define TAC_REMOVE_MULTIPLE_ASSIGN_H

#include <memory>
#include <unordered_set>

#include <boost/variant/static_visitor.hpp>

#include "tac/Quadruple.hpp"
#include "tac/IfFalse.hpp"
#include "tac/If.hpp"

#include "tac/Optimizer.hpp"
#include "tac/OptimizerUtils.hpp"

namespace eddic {

namespace tac {

class RemoveMultipleAssign : public boost::static_visitor<bool> {
    public:
        bool optimized;
        Pass pass;

        RemoveMultipleAssign() : optimized(false) {}

        bool operator()(std::shared_ptr<tac::Quadruple>& quadruple);
        bool operator()(std::shared_ptr<tac::IfFalse>& ifFalse);
        bool operator()(std::shared_ptr<tac::If>& if_);

        template<typename T>
        bool operator()(T&) const { 
            return false;
        }
    
        void collect(tac::Argument* arg);
        void collect(boost::optional<tac::Argument>& arg);

    private:
        std::unordered_set<std::shared_ptr<Variable>> used;
        std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<tac::Quadruple>> lastAssign;
        std::unordered_set<std::shared_ptr<tac::Quadruple>> removed;
};


} //end of tac

} //end of eddic

#endif
