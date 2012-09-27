//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/Pass.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TemplateEngine.hpp"

using namespace eddic;
        
void ast::Pass::set_template_engine(std::shared_ptr<ast::TemplateEngine> template_engine){
    this->template_engine = template_engine;
}

void ast::Pass::set_platform(Platform platform){
    this->platform = platform;
}

void ast::Pass::set_configuration(std::shared_ptr<Configuration> configuration){
    this->configuration = configuration;
}

bool ast::Pass::is_simple(){
    return false;
}
        
void ast::Pass::apply_program(ast::SourceFile&, bool){
    //Do nothing by default
}

void ast::Pass::apply_function(ast::FunctionDeclaration&){
    //Do nothing by default
}

void ast::Pass::apply_struct(ast::Struct&, bool){
    //Do nothing by default
}

void ast::Pass::apply_struct_function(ast::FunctionDeclaration&){
    //Do nothing by default
}

void ast::Pass::apply_struct_constructor(ast::Constructor&){
    //Do nothing by default
}

void ast::Pass::apply_struct_destructor(ast::Destructor&){
    //Do nothing by default
}
