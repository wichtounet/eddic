//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef TRANSFORMER_ENGINE_H
#define TRANSFORMER_ENGINE_H

#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct CleanPass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;
    bool is_simple() override;
};

struct TransformPass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;
    bool is_simple() override;
};

} //end of ast

} //end of eddic

#endif
