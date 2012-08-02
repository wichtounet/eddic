//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_PARAM_H
#define MTAC_PARAM_H

#include <memory>
#include <string>
#include <vector>

#include "mtac/Argument.hpp"

namespace eddic {

class Variable;

namespace mtac {

struct Param {
    mtac::Argument arg;

    std::shared_ptr<Variable> param;
    std::string std_param;
    std::shared_ptr<Function> function;
    
    bool address = false;
    std::vector<std::string> memberNames; //Only valid when address == true

    Param(const Param& rhs) = delete;
    Param& operator=(const Param& rhs) = delete;

    Param();
    Param(mtac::Argument arg1);
    Param(mtac::Argument arg1, std::shared_ptr<Variable> param, std::shared_ptr<Function> function);
    Param(mtac::Argument arg1, const std::string& param, std::shared_ptr<Function> function);
};

} //end of mtac

} //end of eddic

#endif
