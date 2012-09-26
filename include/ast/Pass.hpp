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

class Pass {
    public:
        virtual void apply_program(ast::SourceFile& program) = 0;
        virtual void apply_struct(ast::Struct& struct_) = 0;
        virtual void apply_function(ast::FunctionDeclaration& function) = 0;

        bool simple = false;
};

} //end of ast

} //end of eddic

#endif
