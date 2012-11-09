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

#include "ast/structure_collection.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

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
