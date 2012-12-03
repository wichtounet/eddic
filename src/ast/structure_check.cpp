//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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

void ast::StructureCheckPass::apply_struct(ast::Struct& struct_, bool indicator){
    if(indicator){
        return;
    }

    auto struct_type = context->get_struct(struct_.Content->struct_type->mangle());

    for(auto& block : struct_.Content->blocks){
        if(auto* ptr = boost::get<ast::MemberDeclaration>(&block)){
            auto& member = *ptr;

            auto type = (*struct_type)[member.Content->name]->type;

            if(type->is_custom_type()){
                auto struct_name = type->mangle();

                if(!context->struct_exists(struct_name)){
                    throw SemanticalException("Invalid member type " + struct_name, member.Content->position);
                }
            }
        } else if(auto* ptr = boost::get<ast::ArrayDeclaration>(&block)){
            auto& member = *ptr;

            auto type = (*struct_type)[member.Content->arrayName]->type;

            if(type->is_custom_type()){
                auto struct_name = type->mangle();

                if(!context->struct_exists(struct_name)){
                    throw SemanticalException("Invalid member type " + struct_name, member.Content->position);
                }
            }
        }
    }
}
