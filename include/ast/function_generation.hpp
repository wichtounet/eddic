//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_FUNCTION_GENERATION_PASS_H
#define AST_FUNCTION_GENERATION_PASS_H

#include "ast/ContextAwarePass.hpp"

namespace eddic {

namespace ast {

struct FunctionGenerationPass : ContextAwarePass {
    void apply_struct(ast::struct_definition& struct_, bool indicator) override;
};

} //end of ast

} //end of eddic

#endif
