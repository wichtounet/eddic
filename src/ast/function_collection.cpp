//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
        
void ast::FunctionCollectionPass::apply_function(ast::FunctionDeclaration& declaration){
    auto return_type = visit(ast::TypeTransformer(context), declaration.Content->returnType);

    if(return_type->is_array()){
        throw SemanticalException("Cannot return array from function", declaration.Content->position);
    }

    std::vector<ParameterType> parameters;
    for(auto& param : declaration.Content->parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        parameters.emplace_back(param.parameterName, paramType);
    }
    
    auto mangled_name = mangle(declaration.Content->functionName, parameters, declaration.Content->struct_type);

    if(context->exists(mangled_name)){
        throw SemanticalException("The function " + mangled_name + " has already been defined", declaration.Content->position);
    }

    auto& signature = context->add_function(return_type, declaration.Content->functionName, mangled_name);
    
    LOG<Info>("Functions") << "Register function " << mangled_name << log::endl;

    signature.struct_type = declaration.Content->struct_type;
    signature.context = declaration.Content->context;
    signature.parameters() = std::move(parameters);

    declaration.Content->mangledName = mangled_name;

    //Return by value needs a new parameter on stack
    if(return_type->is_custom_type() || return_type->is_template_type()){
        signature.parameters().emplace_back("__ret", new_pointer_type(return_type));
    }
}

void ast::FunctionCollectionPass::apply_struct_function(ast::FunctionDeclaration& function){
   apply_function(function); 
}

void ast::FunctionCollectionPass::apply_struct_constructor(ast::Constructor& constructor){
    std::vector<ParameterType> parameters;
    for(auto& param : constructor.Content->parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        parameters.emplace_back(param.parameterName, paramType);
    }
    
    auto mangled_name = mangle_ctor(parameters, constructor.Content->struct_type);

    if(context->exists(mangled_name)){
        throw SemanticalException("The constructor " + mangled_name + " has already been defined", constructor.Content->position);
    }
    
    auto& signature = context->add_function(VOID, "ctor", mangled_name);

    signature.struct_type = constructor.Content->struct_type;
    signature.context = constructor.Content->context;
    signature.parameters() = std::move(parameters);

    constructor.Content->mangledName = mangled_name;
}

void ast::FunctionCollectionPass::apply_struct_destructor(ast::Destructor& destructor){
    //This is necessary to collect the "this" parameter
    std::vector<ParameterType> parameters;
    for(auto& param : destructor.Content->parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        parameters.emplace_back(param.parameterName, paramType);
    }
    
    auto mangled_name = mangle_dtor(destructor.Content->struct_type);

    if(context->exists(mangled_name)){
        throw SemanticalException("Only one destructor per struct is allowed", destructor.Content->position);
    }

    auto& signature = context->add_function(VOID, "dtor", mangled_name);

    signature.struct_type = destructor.Content->struct_type;
    signature.context = destructor.Content->context;
    signature.parameters() = std::move(parameters);

    destructor.Content->mangledName = mangled_name;
}
