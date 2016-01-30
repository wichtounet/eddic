//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
