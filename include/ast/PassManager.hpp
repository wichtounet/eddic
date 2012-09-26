//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PASS_MANAGER_H
#define AST_PASS_MANAGER_H

#include <memory>
#include <vector>

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

class Pass;
class TemplateEngine;

class PassManager {
    public:
        PassManager(ast::TemplateEngine& template_engine);

        void init_passes();
        void run_passes(ast::SourceFile& program);

    private:
        ast::TemplateEngine& template_engine;

        std::vector<std::shared_ptr<Pass>> passes;
        std::vector<std::shared_ptr<Pass>> applied_passes;
};

} //end of ast

} //end of eddic

#endif
