//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"

#include "ast/TemplateCollectionPass.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TemplateEngine.hpp"

using namespace eddic;

namespace {

struct Collector : public boost::static_visitor<> {
    ast::TemplateEngine::FunctionTemplateMap& function_templates;
    ast::TemplateEngine::ClassTemplateMap& class_templates;

    std::string parent_struct = "";

    Collector(ast::TemplateEngine::FunctionTemplateMap& function_templates, ast::TemplateEngine::ClassTemplateMap& class_templates) : 
            function_templates(function_templates), class_templates(class_templates) {}

    AUTO_RECURSE_PROGRAM()

    void operator()(ast::TemplateFunctionDeclaration& declaration){
        log::emit<Trace>("Template") << "Collected function template " << declaration.Content->functionName <<" in context " << parent_struct << log::endl;

        function_templates[parent_struct].insert(ast::TemplateEngine::LocalFunctionTemplateMap::value_type(declaration.Content->functionName, declaration));
    }
    
    void operator()(ast::TemplateStruct& template_struct){
        class_templates.insert(ast::TemplateEngine::ClassTemplateMap::value_type(template_struct.Content->name, template_struct)); 
    }
        
    void operator()(ast::Struct& struct_){
        parent_struct = struct_.Content->struct_type->mangle();

        visit_each_non_variant(*this, struct_.Content->template_functions);

        parent_struct = "";
    }

    AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

ast::TemplateCollectionPass::TemplateCollectionPass(ast::TemplateEngine& template_engine) : Pass(template_engine) {}

bool ast::TemplateCollectionPass::is_simple(){
    return true;
}

void ast::TemplateCollectionPass::apply_program(ast::SourceFile& program){
    Collector collector(template_engine.function_templates, template_engine.class_templates);
    collector(program);
}
