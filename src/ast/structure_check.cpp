//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <string>

#include "SemanticalException.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "mangling.hpp"
#include "VisitorUtils.hpp"

#include "ast/structure_check.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

void ast::StructureCheckPass::apply_struct(ast::struct_definition& struct_, bool indicator){
    if(indicator){
        return;
    }

    auto struct_type = context->get_struct(struct_.struct_type);

    for(auto& block : struct_.blocks){
        if(auto* ptr = boost::get<ast::MemberDeclaration>(&block)){
            auto& member = *ptr;

            auto type = (*struct_type)[member.name].type;

            if(type->is_custom_type()){
                if(!context->struct_exists(type)){
                    context->error_handler.semantical_exception("Invalid member type " + type->mangle(), member);
                }
            }
        } else if(auto* ptr = boost::get<ast::ArrayDeclaration>(&block)){
            auto& member = *ptr;

            auto type = (*struct_type)[member.arrayName].type;

            if(type->is_custom_type()){
                if(!context->struct_exists(type)){
                    context->error_handler.semantical_exception("Invalid member type " + type->mangle(), member);
                }
            }
        }
    }
}
