//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PASS_H
#define AST_PASS_H

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

struct TemplateEngine;

class Pass {
    public:
        Pass(ast::TemplateEngine& template_engine);

        virtual void apply_program(ast::SourceFile& program);
        virtual void apply_function(ast::FunctionDeclaration& function);
        virtual void apply_struct(ast::Struct& struct_);
        virtual void apply_struct_function(ast::FunctionDeclaration& function);
        virtual void apply_struct_constructor(ast::Constructor& constructor);
        virtual void apply_struct_destructor(ast::Destructor& destructor);
        
        virtual bool is_simple() = 0;

    protected:
        ast::TemplateEngine& template_engine;
};

} //end of ast

} //end of eddic

#endif
