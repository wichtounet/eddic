//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEFAULT_VALUES_H
#define DEFAULT_VALUES_H

#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct DefaultValuesPass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;
    void apply_function(ast::FunctionDeclaration& function) override;
    void apply_struct_function(ast::FunctionDeclaration& function) override;
    void apply_struct_constructor(ast::Constructor& constructor) override;
    void apply_struct_destructor(ast::Destructor& destructor) override;
};

} //end of ast

} //end of eddic

#endif
