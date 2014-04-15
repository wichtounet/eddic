//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_CONTEXT_AWARE_PASS_H
#define AST_CONTEXT_AWARE_PASS_H

#include "ast/Pass.hpp"

namespace eddic {

struct GlobalContext;

namespace ast {

struct ContextAwarePass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;

    std::shared_ptr<GlobalContext> context;
};

} //end of ast

} //end of eddic

#endif
