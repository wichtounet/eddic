//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_REMOVE_ASSIGN_H
#define MTAC_REMOVE_ASSIGN_H

#include <memory>
#include <unordered_set>

#include "variant.hpp"

#include "mtac/Pass.hpp"
#include "mtac/forward.hpp"

namespace eddic {

class Variable;

namespace mtac {

class RemoveAssign : public boost::static_visitor<void> {
    public:
        bool optimized = false;
        Pass pass;

        void operator()(std::shared_ptr<mtac::Quadruple> quadruple);
        void operator()(std::shared_ptr<mtac::Param> param);
        void operator()(std::shared_ptr<mtac::IfFalse> ifFalse);
        void operator()(std::shared_ptr<mtac::If> if_);

        template<typename T>
        void operator()(T&) const { 
            //Nothing to optimize here
        }
    
    private:
        std::unordered_set<std::shared_ptr<Variable>> used;
};

} //end of mtac

} //end of eddic

#endif
