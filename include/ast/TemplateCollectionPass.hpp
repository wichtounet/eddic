//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TEMPLATE_COLLECTION_PASS_H
#define TEMPLATE_COLLECTION_PASS_H

#include "ast/source_def.hpp"
#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct TemplateCollectionPass : Pass {
    TemplateCollectionPass(ast::TemplateEngine& template_engine);

    void apply_program(ast::SourceFile& program) override;
    bool is_simple() override;
};

} //end of ast

} //end of eddic

#endif
