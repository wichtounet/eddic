//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STRUCTURE_CHECK_PASS_H
#define AST_STRUCTURE_CHECK_PASS_H

#include "ast/ContextAwarePass.hpp"

namespace eddic {

namespace ast {

struct StructureCheckPass : ContextAwarePass {
    void apply_struct(ast::Struct& struct_, bool indicator) override;
};

} //end of ast

} //end of eddic

#endif
