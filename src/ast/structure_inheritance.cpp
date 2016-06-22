//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <unordered_set>

#include "GlobalContext.hpp"
#include "VisitorUtils.hpp"
#include "SemanticalException.hpp"

#include "ast/structure_inheritance.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

void ast::StructureInheritancePass::apply_program(ast::SourceFile& program, bool){
    auto context = program.context;

    std::unordered_set<std::string> resolved_structures;

    while(true){
        auto start_size = resolved_structures.size();
        std::size_t structures = 0;

        for(auto& block : program.blocks){
            if(auto* ptr = boost::get<ast::struct_definition>(&block)){
                if(ptr->is_template_declaration()){
                    continue;
                }

                ++structures;

                auto type = ptr->struct_type;
                auto struct_type = context->get_struct(type);

                //If already resolved
                if(struct_type->parent_type){
                    resolved_structures.insert(type->mangle());
                    continue;
                }

                //If no parent type, already resolved
                if(!ptr->parent_type){
                    resolved_structures.insert(type->mangle());
                    continue;
                }

                auto parent_type = visit(ast::TypeTransformer(context), *ptr->parent_type);

                if(!context->struct_exists(parent_type)){
                    throw SemanticalException("The parent type is not a valid structure type", ptr->position);
                }

                if(resolved_structures.find(parent_type->mangle()) != resolved_structures.end()){
                    struct_type->parent_type = parent_type;
                    resolved_structures.insert(type->mangle());
                }
            }
        }

        if(resolved_structures.size() == start_size){
            if(resolved_structures.size() != structures){
                throw SemanticalException("Invalid inheritance tree");
            } else {
                break;
            }
        }
    }
}
