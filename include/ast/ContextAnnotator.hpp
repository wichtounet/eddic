//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONTEXT_ANNOTATOR_H
#define CONTEXT_ANNOTATOR_H

#include <memory>

#include "ast/source_def.hpp"
#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

class ContextAnnotationPass : public Pass {
    public:
        void apply_program(ast::SourceFile& program, bool indicator) override;
        void apply_function(ast::FunctionDeclaration& function) override;
        void apply_struct_function(ast::FunctionDeclaration& function) override;
        void apply_struct_constructor(ast::Constructor& constructor) override;
        void apply_struct_destructor(ast::Destructor& destructor) override;

    private:
        std::shared_ptr<GlobalContext> globalContext;
        std::shared_ptr<FunctionContext> functionContext;
        std::shared_ptr<Context> currentContext;
};

} //end of ast

} //end of eddic

#endif
