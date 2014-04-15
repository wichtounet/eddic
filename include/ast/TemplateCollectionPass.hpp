//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef TEMPLATE_COLLECTION_PASS_H
#define TEMPLATE_COLLECTION_PASS_H

#include "ast/Pass.hpp"

namespace eddic {

namespace ast {

struct TemplateCollectionPass : Pass {
    void apply_program(ast::SourceFile& program, bool indicator) override;
    void apply_struct(ast::Struct& struct_, bool indicator) override;
};

} //end of ast

} //end of eddic

#endif
