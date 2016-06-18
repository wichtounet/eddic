//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
void annotate(T& declaration, ast::struct_definition& current_struct){
    declaration.struct_type = current_struct.Content->struct_type;

    ast::PointerType paramType;

    if(current_struct.Content->is_template_instantation()){
        ast::TemplateType struct_type;
        struct_type.type = current_struct.Content->name;
        struct_type.template_types = current_struct.Content->inst_template_types;

        paramType.type = struct_type;
    } else {
        ast::SimpleType struct_type;
        struct_type.type = current_struct.Content->name;
        struct_type.const_ = false;

        paramType.type = struct_type;
    }

    ast::FunctionParameter param;
    param.parameterName = "this";
    param.parameterType = paramType;

    declaration.parameters.insert(declaration.parameters.begin(), param);
}

} //end of anonymous namespace

void ast::MemberFunctionCollectionPass::apply_struct(ast::struct_definition& struct_, bool){
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
