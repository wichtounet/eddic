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

#include "Platform.hpp"

#include "ast/source_def.hpp"
#include "ast/VariableType.hpp"
#include "ast/TemplateEngine.hpp"

namespace eddic {

class Configuration;

namespace ast {

class Pass;

class PassManager {
    public:
        PassManager(Platform platform, std::shared_ptr<Configuration> configuration);

        void init_passes();
        void run_passes(ast::SourceFile& program);

    private:
        std::shared_ptr<ast::TemplateEngine> template_engine;
        Platform platform;
        std::shared_ptr<Configuration> configuration;

        std::vector<std::shared_ptr<Pass>> passes;
        std::vector<std::shared_ptr<Pass>> applied_passes;
};

} //end of ast

} //end of eddic

#endif
