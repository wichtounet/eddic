//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRING_CHECKER_H
#define STRING_CHECKER_H

#include "ast/Program.hpp"

namespace eddic {

class StringPool;

struct StringChecker {
   void check(ASTProgram& program, StringPool& pool);
};

} //end of eddic

#endif
