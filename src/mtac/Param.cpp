//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"

#include "mtac/Param.hpp"

using namespace eddic;

mtac::Param::Param(mtac::Argument a, std::shared_ptr<Variable> param, Function& function) : arg(a), param(param), function(function) {
    eddic_assert(param, "The parameter variable cannot be null");    
    eddic_assert(function, "The function cannot be null");    
}

mtac::Param::Param(mtac::Argument a, const std::string& param, Function& function) : arg(a), std_param(param), function(function) {
    eddic_assert(function, "The function cannot be null");    
}
