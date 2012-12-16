//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <memory>

#include "variant.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "mangling.hpp"

#include "ast/VariablesAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/VariableType.hpp"
#include "ast/TemplateEngine.hpp"

using namespace eddic;

namespace {

struct VariablesVisitor : public boost::static_visitor<> {
    std::shared_ptr<GlobalContext> context;
    std::shared_ptr<ast::TemplateEngine> template_engine;

    VariablesVisitor(std::shared_ptr<GlobalContext> context, std::shared_ptr<ast::TemplateEngine> template_engine) : 
            context(context), template_engine(template_engine) {
                //NOP
    }

    AUTO_RECURSE_RETURN_VALUES()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_SWITCH()
    AUTO_RECURSE_SWITCH_CASE()
    AUTO_RECURSE_DEFAULT_CASE()
    AUTO_RECURSE_NEW_ARRAY()
    AUTO_RECURSE_PREFIX()
    AUTO_RECURSE_COMPOSED_VALUES()
    AUTO_RECURSE_BUILTIN_OPERATORS()
    AUTO_RECURSE_TERNARY()
    AUTO_RECURSE_CAST_VALUES()
    AUTO_RECURSE_NEW()
    AUTO_RECURSE_FUNCTION_CALLS()

    void operator()(ast::Assignment& assignment){
        visit(*this, assignment.Content->left_value);
        visit(*this, assignment.Content->value);
    }

    template<typename Function>
    void visit_function(Function& declaration){
        //Add all the parameters to the function context
        for(auto& parameter : declaration.Content->parameters){
            template_engine->check_type(parameter.parameterType, declaration.Content->position);
        }

        visit_each(*this, declaration.Content->instructions);
    }

    void operator()(ast::GlobalVariableDeclaration& declaration){
        template_engine->check_type(declaration.Content->variableType, declaration.Content->position);
    }

    void operator()(ast::GlobalArrayDeclaration& declaration){
        template_engine->check_type(declaration.Content->arrayType, declaration.Content->position);
    }
    
    void operator()(ast::ArrayDeclaration& declaration){
        template_engine->check_type(declaration.Content->arrayType, declaration.Content->position);
    }
    
    void operator()(ast::Foreach& foreach){
        template_engine->check_type(foreach.Content->variableType, foreach.Content->position);

        visit_each(*this, foreach.Content->instructions);
    }
    
    void operator()(ast::ForeachIn& foreach){
        template_engine->check_type(foreach.Content->variableType, foreach.Content->position);

        visit_each(*this, foreach.Content->instructions);
    }
    
    void operator()(ast::StructDeclaration& declaration){
        template_engine->check_type(declaration.Content->variableType, declaration.Content->position);
    }
    
    void operator()(ast::VariableDeclaration& declaration){
        template_engine->check_type(declaration.Content->variableType, declaration.Content->position);
    }
    
    AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace
    
void ast::VariableAnnotationPass::apply_function(ast::FunctionDeclaration& function){
    VariablesVisitor visitor(context, template_engine);
    visitor.visit_function(function);
}

void ast::VariableAnnotationPass::apply_struct_function(ast::FunctionDeclaration& function){
    VariablesVisitor visitor(context, template_engine);
    visitor.visit_function(function);
}

void ast::VariableAnnotationPass::apply_struct_constructor(ast::Constructor& constructor){
    VariablesVisitor visitor(context, template_engine);
    visitor.visit_function(constructor);
}

void ast::VariableAnnotationPass::apply_struct_destructor(ast::Destructor& destructor){
    VariablesVisitor visitor(context, template_engine);
    visitor.visit_function(destructor);
}

void ast::VariableAnnotationPass::apply_program(ast::SourceFile& program, bool indicator){
    context = program.Content->context;

    if(!indicator){
        VariablesVisitor visitor(context, template_engine);

        for(auto& block : program.Content->blocks){
            if(auto* ptr = boost::get<ast::GlobalArrayDeclaration>(&block)){
                visit_non_variant(visitor, *ptr);
            } else if(auto* ptr = boost::get<ast::GlobalVariableDeclaration>(&block)){
                visit_non_variant(visitor, *ptr);
            }
        }
    }
}
