//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef VARIABLES_ANNOTATOR_H
#define VARIABLES_ANNOTATOR_H

#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct VariableAnnotationPass : Pass {
    void apply_function(ast::FunctionDeclaration& function) override;
    void apply_struct_function(ast::FunctionDeclaration& function) override;
    void apply_struct_constructor(ast::Constructor& constructor) override;
    void apply_struct_destructor(ast::Destructor& destructor) override;
    void apply_program(ast::SourceFile& program, bool indicator) override;

    std::shared_ptr<GlobalContext> context;
};

} //end of ast

} //end of eddic

#endif
