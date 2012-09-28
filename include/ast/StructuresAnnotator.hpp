//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRUCTURES_ANNOTATOR_H
#define STRUCTURES_ANNOTATOR_H

#include "ast/source_def.hpp"
#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct StructureCollectionPass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;
    void apply_struct(ast::Struct& struct_, bool indicator) override;

    std::shared_ptr<GlobalContext> context;
};

} //end of ast

} //end of eddic

#endif
