//=======================================================================
// Copyright Baptiste Wicht 2011.
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

#include "ast/StructuresAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

void ast::StructureCollectionPass::apply_program(ast::SourceFile& program, bool){
    context = program.Content->context;
}

void ast::StructureCollectionPass::apply_struct(ast::Struct& struct_, bool indicator){
    if(indicator){
        return;
    }

    if(struct_.Content->template_types.empty()){
        struct_.Content->struct_type = new_type(context, struct_.Content->name, false);
    } else {
        std::vector<std::shared_ptr<const eddic::Type>> template_types;

        ast::TypeTransformer transformer(context);

        for(auto& type : struct_.Content->template_types){
            template_types.push_back(visit(transformer, type));
        }

        struct_.Content->struct_type = new_template_type(context, struct_.Content->name, template_types);
    }

    //Annotate functions with the parent struct
    for(auto& function : struct_.Content->functions){
        if(function.Content->context){
            function.Content->context->struct_type = struct_.Content->struct_type; 
        }
    }

    for(auto& function : struct_.Content->constructors){
        if(function.Content->context){
            function.Content->context->struct_type = struct_.Content->struct_type; 
        }
    }

    for(auto& function : struct_.Content->destructors){
        if(function.Content->context){
            function.Content->context->struct_type = struct_.Content->struct_type; 
        }
    }

    auto mangled_name = struct_.Content->struct_type->mangle();

    if(context->struct_exists(mangled_name)){
        throw SemanticalException("The structure " + mangled_name + " has already been defined", struct_.Content->position);
    }

    auto signature = std::make_shared<eddic::Struct>(mangled_name);
    context->add_struct(signature);
}

void ast::StructureMemberCollectionPass::apply_program(ast::SourceFile& program, bool){
    context = program.Content->context;
}

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

void ast::StructureCheckPass::apply_program(ast::SourceFile& program, bool){
    context = program.Content->context;
}

void ast::StructureCheckPass::apply_struct(ast::Struct& struct_, bool indicator){
    if(indicator){
        return;
    }

    auto struct_type = context->get_struct(struct_.Content->struct_type->mangle());

    for(auto& member : struct_.Content->members){
        auto type = (*struct_type)[member.Content->name]->type;

        if(type->is_custom_type()){
            auto struct_name = type->mangle();

            if(!context->struct_exists(struct_name)){
                throw SemanticalException("Invalid member type " + struct_name, member.Content->position);
            }
        }
    }
}
