//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
