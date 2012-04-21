//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_RETURN_H
#define TAC_RETURN_H

#include <unordered_map>

#include <boost/optional.hpp>

#include "mtac/Argument.hpp"

namespace eddic {

namespace mtac {

struct Return {
    boost::optional<Argument> arg1;
    boost::optional<Argument> arg2;

    std::unordered_map<std::shared_ptr<Variable>, bool> liveness;

    Return();
    Return(Argument arg1);
    Return(Argument arg1, Argument arg2);
};

} //end of mtac

} //end of eddic

#endif
