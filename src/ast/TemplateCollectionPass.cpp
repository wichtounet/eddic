//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ast/TemplateCollectionPass.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TemplateEngine.hpp"

using namespace eddic;

namespace {

struct Collector : public boost::static_visitor<> {
    ast::TemplateEngine& template_engine;

    std::string parent_struct = "";

    Collector(ast::TemplateEngine& template_engine) : template_engine(template_engine) {}

    AUTO_RECURSE_PROGRAM()

    void operator()(ast::TemplateFunctionDeclaration& declaration){
        template_engine.add_template_function(parent_struct, declaration.Content->functionName, declaration);
    }
    
    void operator()(ast::TemplateStruct& template_struct){
        template_engine.add_template_struct(template_struct.Content->name, template_struct);
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
    Collector collector(template_engine);
    collector(program);
}
