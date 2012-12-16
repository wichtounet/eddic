//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
    auto context = program.Content->context;

    std::unordered_set<std::string> resolved_structures;

    while(true){
        auto start_size = resolved_structures.size();
        std::size_t structures = 0;

        for(auto& block : program.Content->blocks){
            if(auto* ptr = boost::get<ast::Struct>(&block)){
                ++structures;

                auto type = ptr->Content->struct_type;
                auto struct_type = context->get_struct(type);

                //If already resolved
                if(struct_type->parent_type){
                    resolved_structures.insert(type->mangle()); 
                    continue;
                }

                //If no parent type, already resolved
                if(!ptr->Content->parent_type){
                    resolved_structures.insert(type->mangle()); 
                    continue;
                }

                auto parent_type = visit(ast::TypeTransformer(context), *ptr->Content->parent_type);

                if(!context->struct_exists(parent_type)){
                    throw SemanticalException("The parent type is not a valid structure type", ptr->Content->position);
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
