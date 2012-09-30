//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PASS_H
#define AST_PASS_H

#include <memory>

#include "Platform.hpp"

#include "ast/source_def.hpp"

namespace eddic {

struct Configuration;

namespace ast {

struct TemplateEngine;

class Pass {
    public:
        virtual void apply_program(ast::SourceFile& program, bool indicator);
        virtual void apply_function(ast::FunctionDeclaration& function);
        virtual void apply_struct(ast::Struct& struct_, bool indicator);
        virtual void apply_struct_function(ast::FunctionDeclaration& function);
        virtual void apply_struct_constructor(ast::Constructor& constructor);
        virtual void apply_struct_destructor(ast::Destructor& destructor);

        void set_template_engine(std::shared_ptr<ast::TemplateEngine> template_engine);
        void set_platform(Platform platform);
        void set_configuration(std::shared_ptr<Configuration> configuration);
        void set_current_pass(unsigned int i);
        void set_name(const std::string& name);
        std::string name();
        
        virtual unsigned int passes();
        virtual bool is_simple();

    protected:
        unsigned int pass = 0;
        std::string pass_name;

        std::shared_ptr<ast::TemplateEngine> template_engine;
        Platform platform;
        std::shared_ptr<Configuration> configuration;
};

} //end of ast

} //end of eddic

#endif
