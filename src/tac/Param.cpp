//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/Param.hpp"

using namespace eddic;

tac::Param::Param(){}
tac::Param::Param(tac::Argument a) : arg(a) {}
tac::Param::Param(tac::Argument a, std::shared_ptr<Variable> param, std::shared_ptr<Function> function) : arg(a), param(param), function(function) {}
tac::Param::Param(tac::Argument a, const std::string& param, std::shared_ptr<Function> function) : arg(a), std_param(param), function(function) {}
