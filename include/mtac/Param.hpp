//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_PARAM_H
#define MTAC_PARAM_H

#include <memory>
#include <string>

#include "mtac/Argument.hpp"

namespace eddic {

class Variable;
class Function;

namespace mtac {

struct Param {
    mtac::Argument arg;

    std::shared_ptr<Variable> param;
    std::string std_param;
    eddic::Function& function;
    unsigned int depth;
    
    bool address = false;

    Param(const Param& rhs) = delete;
    Param& operator=(const Param& rhs) = delete;

    Param(mtac::Argument arg1, std::shared_ptr<Variable> param, eddic::Function& function);
    Param(mtac::Argument arg1, const std::string& param, eddic::Function& function);
};

} //end of mtac

} //end of eddic

#endif
