//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "SemanticalException.hpp"
#include "mangling.hpp"
#include "Options.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "logging.hpp"

#include "ast/function_collection.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"

using namespace eddic;

void ast::FunctionCollectionPass::apply_function(ast::TemplateFunctionDeclaration& declaration){
    auto return_type = visit(ast::TypeTransformer(context), declaration.returnType);

    if(return_type->is_array()){
        throw SemanticalException("Cannot return array from function", declaration.position);
    }

    std::vector<Parameter> parameters;
    for(auto& param : declaration.parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        parameters.emplace_back(param.parameterName, paramType);
    }

    auto mangled_name = mangle(declaration.functionName, parameters, declaration.struct_type);

    if(context->exists(mangled_name)){
        throw SemanticalException("The function " + mangled_name + " has already been defined", declaration.position);
    }

    auto& signature = context->add_function(return_type, declaration.functionName, mangled_name);

    LOG<Info>("Functions") << "Register function " << mangled_name << log::endl;

    signature.struct_type() = declaration.struct_type;
    signature.context() = declaration.context;
    signature.parameters() = std::move(parameters);

    declaration.mangledName = mangled_name;

    //Return by value needs a new parameter on stack
    if(return_type->is_custom_type() || return_type->is_template_type()){
        signature.parameters().emplace_back("__ret", new_pointer_type(return_type));
    }
}

void ast::FunctionCollectionPass::apply_struct_function(ast::TemplateFunctionDeclaration& function){
   apply_function(function);
}

void ast::FunctionCollectionPass::apply_struct_constructor(ast::Constructor& constructor){
    std::vector<Parameter> parameters;
    for(auto& param : constructor.parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        parameters.emplace_back(param.parameterName, paramType);
    }

    auto mangled_name = mangle_ctor(parameters, constructor.struct_type);

    if(context->exists(mangled_name)){
        throw SemanticalException("The constructor " + mangled_name + " has already been defined", constructor.position);
    }

    auto& signature = context->add_function(VOID, "ctor", mangled_name);

    signature.struct_type() = constructor.struct_type;
    signature.context() = constructor.context;
    signature.parameters() = std::move(parameters);

    constructor.mangledName = mangled_name;
}

void ast::FunctionCollectionPass::apply_struct_destructor(ast::Destructor& destructor){
    //This is necessary to collect the "this" parameter
    std::vector<Parameter> parameters;
    for(auto& param : destructor.parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        parameters.emplace_back(param.parameterName, paramType);
    }

    auto mangled_name = mangle_dtor(destructor.struct_type);

    if(context->exists(mangled_name)){
        throw SemanticalException("Only one destructor per struct is allowed", destructor.position);
    }

    auto& signature = context->add_function(VOID, "dtor", mangled_name);

    signature.struct_type() = destructor.struct_type;
    signature.context() = destructor.context;
    signature.parameters() = std::move(parameters);

    destructor.mangledName = mangled_name;
}
