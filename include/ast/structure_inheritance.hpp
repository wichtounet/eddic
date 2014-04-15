//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_STRUCTURE_INHERITANCE_PASS_H
#define AST_STRUCTURE_INHERITANCE_PASS_H

#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct StructureInheritancePass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator);
};

} //end of ast

} //end of eddic

#endif
