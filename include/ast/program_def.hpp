//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PROGRAM_DEF_H
#define AST_PROGRAM_DEF_H

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {
    struct ASTProgram;

    typedef Deferred<ASTProgram> Program;
} //end of ast

} //end of eddic

#endif
