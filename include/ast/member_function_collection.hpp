//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_MEMBER_FUNCTION_COLLECTION_PASS_H
#define AST_MEMBER_FUNCTION_COLLECTION_PASS_H

#include "ast/ContextAwarePass.hpp"

namespace eddic {

namespace ast {

struct MemberFunctionCollectionPass : Pass {
    void apply_struct(ast::struct_definition& struct_, bool indicator) override;
    void apply_struct_function(ast::TemplateFunctionDeclaration& function) override;
    void apply_struct_constructor(ast::Constructor& constructor) override;
    void apply_struct_destructor(ast::Destructor& destructor) override;

    ast::struct_definition* current_struct;
};

} //end of ast

} //end of eddic

#endif
