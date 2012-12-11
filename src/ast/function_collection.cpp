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
    auto signature = std::make_shared<Function>(return_type, declaration.Content->functionName);

    if(return_type->is_array()){
        throw SemanticalException("Cannot return array from function", declaration.Content->position);
    }

    for(auto& param : declaration.Content->parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        signature->parameters.emplace_back(param.parameterName, paramType);
    }

    //Return by value needs a new parameter on stack
    if(return_type->is_custom_type()){
        signature->parameters.emplace_back("__ret", new_pointer_type(return_type));
    }

    signature->struct_ = declaration.Content->struct_name;
    signature->struct_type = declaration.Content->struct_type;
    signature->context = declaration.Content->context;

    declaration.Content->mangledName = signature->mangledName = mangle(signature);

    if(context->exists(signature->mangledName)){
        throw SemanticalException("The function " + signature->mangledName + " has already been defined", declaration.Content->position);
    }
    
    log::emit<Info>("Functions") << "Register function " << signature->mangledName << log::endl;

    context->addFunction(signature);
}

void ast::FunctionCollectionPass::apply_struct_function(ast::FunctionDeclaration& function){
   apply_function(function); 
}

void ast::FunctionCollectionPass::apply_struct_constructor(ast::Constructor& constructor){
    auto signature = std::make_shared<Function>(VOID, "ctor");

    for(auto& param : constructor.Content->parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        signature->parameters.emplace_back(param.parameterName, paramType);
    }

    signature->struct_ = constructor.Content->struct_name;
    signature->struct_type = constructor.Content->struct_type;

    constructor.Content->mangledName = signature->mangledName = mangle_ctor(signature);

    if(context->exists(signature->mangledName)){
        throw SemanticalException("The constructor " + signature->name + " has already been defined", constructor.Content->position);
    }

    context->addFunction(signature);
    context->getFunction(signature->mangledName)->context = constructor.Content->context;
}

void ast::FunctionCollectionPass::apply_struct_destructor(ast::Destructor& destructor){
    auto signature = std::make_shared<Function>(VOID, "dtor");

    for(auto& param : destructor.Content->parameters){
        auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
        signature->parameters.emplace_back(param.parameterName, paramType);
    }

    signature->struct_ = destructor.Content->struct_name;
    signature->struct_type = destructor.Content->struct_type;

    destructor.Content->mangledName = signature->mangledName = mangle_dtor(signature);

    if(context->exists(signature->mangledName)){
        throw SemanticalException("Only one destructor per struct is allowed", destructor.Content->position);
    }

    context->addFunction(signature);
    context->getFunction(signature->mangledName)->context = destructor.Content->context;
}
