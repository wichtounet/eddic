//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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

void ast::StructureMemberCollectionPass::apply_struct(ast::struct_definition& struct_, bool indicator){
    if(indicator){
        return;
    }

    auto signature = context->get_struct(struct_.struct_type->mangle());
    std::vector<std::string> names;

    for(auto& block : struct_.blocks){
        if(auto* ptr = boost::get<ast::MemberDeclaration>(&block)){
            auto& member = *ptr;

            if(std::find(names.begin(), names.end(), member.name) != names.end()){
                context->error_handler.semantical_exception("The member " + member.name + " has already been defined", member);
            }

            names.push_back(member.name);

            auto member_type = visit(ast::TypeTransformer(context), member.type);
            signature->members.emplace_back(member.name, member_type);
        } else if(auto* ptr = boost::get<ast::ArrayDeclaration>(&block)){
            auto& member = *ptr;
            auto& name = member.arrayName;

            if(std::find(names.begin(), names.end(), name) != names.end()){
                throw SemanticalException("The member " + name + " has already been defined", member.position);
            }

            names.push_back(name);

            auto data_member_type = visit(ast::TypeTransformer(context), member.arrayType);

            if(data_member_type->is_array()){
                throw SemanticalException("Multidimensional arrays are not permitted", member.position);
            }

            if(auto* ptr = boost::get<ast::Integer>(&member.size)){
                signature->members.emplace_back(name, new_array_type(data_member_type, ptr->value));
            } else {
                throw SemanticalException("Only arrays of fixed size are supported", member.position);
            }
        }
    }

    std::sort(signature->members.begin(), signature->members.end(),
            [](const Member& lhs, const Member& rhs){
                if(lhs.type == CHAR || lhs.type == BOOL){
                    return false;
                } else if(rhs.type == CHAR || rhs.type == BOOL){
                    return true;
                } else {
                    return false;
                }
            });
}
