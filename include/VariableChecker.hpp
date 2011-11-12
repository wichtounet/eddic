//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLE_CHECKER_H
#define VARIABLE_CHECKER_H

#include "ast/Program.hpp"

namespace eddic {

struct VariableChecker {
    void check(ast::Program& program);
};

} //end of eddic

#endif
