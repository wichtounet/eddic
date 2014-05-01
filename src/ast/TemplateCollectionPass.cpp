//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
    
    void operator()(ast::struct_definition& template_struct){
        if(template_struct.Content->is_template_declaration()){
            template_engine.add_template_struct(template_struct.Content->name, template_struct);
        }
    }

    AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

void ast::TemplateCollectionPass::apply_program(ast::SourceFile& program, bool indicator){
    if(!indicator){
        Collector collector(*template_engine);
        collector(program);
    }
}
    
void ast::TemplateCollectionPass::apply_struct(ast::struct_definition& struct_, bool indicator){
    if(!indicator){
        Collector collector(*template_engine);
        collector.parent_struct = struct_.Content->struct_type->mangle();
        visit_each(collector, struct_.Content->blocks);
    }
}
