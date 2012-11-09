//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "SemanticalException.hpp"
#include "VisitorUtils.hpp"
#include "mangling.hpp"
#include "Options.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"

#include "ast/member_function_collection.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/TemplateEngine.hpp"

using namespace eddic;

namespace {

template<typename T>
void annotate(T& declaration, ast::Struct& current_struct){
    declaration.Content->struct_name = current_struct.Content->name;
    declaration.Content->struct_type = current_struct.Content->struct_type;

    ast::PointerType paramType;

    if(current_struct.Content->template_types.empty()){
        ast::SimpleType struct_type;
        struct_type.type = current_struct.Content->name;
        struct_type.const_ = false;

        paramType.type = struct_type;
    } else {
        ast::TemplateType struct_type;
        struct_type.type = current_struct.Content->name;
        struct_type.template_types = current_struct.Content->template_types;

        paramType.type = struct_type;
    }

    ast::FunctionParameter param;
    param.parameterName = "this";
    param.parameterType = paramType;

    declaration.Content->parameters.insert(declaration.Content->parameters.begin(), param);
}

} //end of anonymous namespace
        
void ast::MemberFunctionCollectionPass::apply_struct(ast::Struct& struct_, bool){
    current_struct = struct_;
}
    
void ast::MemberFunctionCollectionPass::apply_struct_function(ast::FunctionDeclaration& function){
    annotate(function, current_struct);
}

void ast::MemberFunctionCollectionPass::apply_struct_constructor(ast::Constructor& constructor){
    annotate(constructor, current_struct);
}

void ast::MemberFunctionCollectionPass::apply_struct_destructor(ast::Destructor& destructor){
    annotate(destructor, current_struct);
}
