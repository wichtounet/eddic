//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef WARNINGS_ENGINE_H
#define WARNINGS_ENGINE_H

#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct WarningsPass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;
    bool is_simple() override;
};

} //end of ast

} //end of eddic

#endif
