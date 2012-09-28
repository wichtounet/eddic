//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTIONS_ANNOTATOR_H
#define FUNCTIONS_ANNOTATOR_H

#include "ast/ContextAwarePass.hpp"

namespace eddic {

namespace ast {

struct MemberFunctionCollectionPass : Pass {
    void apply_struct(ast::Struct& struct_, bool indicator);
    void apply_struct_function(ast::FunctionDeclaration& function) override;
    void apply_struct_constructor(ast::Constructor& constructor) override;
    void apply_struct_destructor(ast::Destructor& destructor) override;

    ast::Struct current_struct;
};

struct FunctionCollectionPass : ContextAwarePass {
    void apply_function(ast::FunctionDeclaration& function) override;
    void apply_struct_function(ast::FunctionDeclaration& function) override;
    void apply_struct_constructor(ast::Constructor& constructor) override;
    void apply_struct_destructor(ast::Destructor& destructor) override;
};

struct FunctionCheckPass : ContextAwarePass {
    void apply_function(ast::FunctionDeclaration& function) override;
    void apply_struct_function(ast::FunctionDeclaration& function) override;
    void apply_struct_constructor(ast::Constructor& constructor) override;
    void apply_struct_destructor(ast::Destructor& destructor) override;
        
    std::shared_ptr<Function> currentFunction;
};

} //end of ast

} //end of eddic

#endif
