//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_CONTEXT_AWARE_PASS_H
#define AST_CONTEXT_AWARE_PASS_H

#include "ast/Pass.hpp"

namespace eddic {

class GlobalContext;

namespace ast {

struct ContextAwarePass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;

    std::shared_ptr<GlobalContext> context;
};

} //end of ast

} //end of eddic

#endif
