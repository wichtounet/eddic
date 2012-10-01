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
class StringPool;

namespace ast {

class Pass;

class PassManager {
    public:
        PassManager(Platform platform, std::shared_ptr<Configuration> configuration, ast::SourceFile& program, std::shared_ptr<StringPool> pool);

        void init_passes();
        void run_passes();

        void function_instantiated(ast::FunctionDeclaration& function, const std::string& context);
        void struct_instantiated(ast::Struct& struct_);

    private:
        std::shared_ptr<ast::TemplateEngine> template_engine;
        Platform platform;
        std::shared_ptr<Configuration> configuration;
        ast::SourceFile& program;
        std::shared_ptr<StringPool> pool;

        std::vector<std::shared_ptr<Pass>> passes;
        std::vector<std::shared_ptr<Pass>> applied_passes;
        
        std::vector<ast::Struct> class_instantiated;
        std::vector<std::pair<std::string, ast::FunctionDeclaration>> functions_instantiated;
};

} //end of ast

} //end of eddic

#endif
