//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct TypeCheckingPass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;
    bool is_simple() override;
};

} //end of ast

} //end of eddic

#endif
