//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_PASS_MANAGER_H
#define AST_PASS_MANAGER_H

#include <memory>
#include <vector>

#include "Platform.hpp"

#include "ast/source_def.hpp"
#include "ast/TemplateEngine.hpp"

namespace eddic {

struct Configuration;
struct StringPool;

namespace ast {

class Pass;

class PassManager {
    public:
        PassManager(Platform platform, std::shared_ptr<Configuration> configuration, ast::SourceFile& program, std::shared_ptr<StringPool> pool);

        void init_passes();
        void run_passes();

        void function_instantiated(ast::FunctionDeclaration& function, const std::string& context);
        void struct_instantiated(ast::struct_definition& struct_);

    private:
        unsigned int template_depth = 0;

        std::shared_ptr<ast::TemplateEngine> template_engine;
        Platform platform;
        std::shared_ptr<Configuration> configuration;
        ast::SourceFile& program;
        std::shared_ptr<StringPool> pool;

        std::vector<std::shared_ptr<Pass>> passes;
        std::vector<std::shared_ptr<Pass>> applied_passes;
        
        std::vector<ast::struct_definition> class_instantiated;
        std::vector<std::pair<std::string, ast::FunctionDeclaration>> functions_instantiated;

        void inc_depth();
        void dec_depth();
};

} //end of ast

} //end of eddic

#endif
