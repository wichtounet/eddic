//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include "SemanticalException.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "mangling.hpp"
#include "VisitorUtils.hpp"

#include "ast/structure_member_collection.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

void ast::StructureMemberCollectionPass::apply_struct(ast::Struct& struct_, bool indicator){
    if(indicator){
        return;
    }

    auto signature = context->get_struct(struct_.Content->struct_type->mangle());
    std::vector<std::string> names;

    for(auto& member : struct_.Content->members){
        if(std::find(names.begin(), names.end(), member.Content->name) != names.end()){
            throw SemanticalException("The member " + member.Content->name + " has already been defined", member.Content->position);
        }

        names.push_back(member.Content->name);

        auto member_type = visit(ast::TypeTransformer(context), member.Content->type);

        if(member_type->is_array()){
            throw SemanticalException("Arrays inside structures are not supported", member.Content->position);
        }

        signature->members.push_back(std::make_shared<Member>(member.Content->name, member_type));
    }
}
