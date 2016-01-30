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

#include "ast/structure_collection.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

void ast::StructureCollectionPass::apply_struct(ast::struct_definition& struct_, bool indicator){
    if(indicator){
        return;
    }

    if(struct_.Content->is_template_instantation()){
        std::vector<std::shared_ptr<const eddic::Type>> template_types;

        ast::TypeTransformer transformer(context);

        for(auto& type : struct_.Content->inst_template_types){
            template_types.push_back(visit(transformer, type));
        }

        struct_.Content->struct_type = new_template_type(context, struct_.Content->name, template_types);
    } else {
        struct_.Content->struct_type = new_type(context, struct_.Content->name, false);
    }

    //Annotate functions with the parent struct
    for(auto& block : struct_.Content->blocks){
        if(auto* ptr = boost::get<ast::FunctionDeclaration>(&block)){
            auto& function = *ptr;

            if(function.Content->context){
                function.Content->context->struct_type = struct_.Content->struct_type; 
            }
        } else if(auto* ptr = boost::get<ast::Constructor>(&block)){
            auto& function = *ptr;

            if(function.Content->context){
                function.Content->context->struct_type = struct_.Content->struct_type; 
            }
        } else if(auto* ptr = boost::get<ast::Destructor>(&block)){
            auto& function = *ptr;

            if(function.Content->context){
                function.Content->context->struct_type = struct_.Content->struct_type; 
            }
        }
    }
    
    auto mangled_name = struct_.Content->struct_type->mangle();

    if(context->struct_exists(mangled_name)){
        throw SemanticalException("The structure " + mangled_name + " has already been defined", struct_.Content->position);
    }

    auto signature = std::make_shared<eddic::Struct>(mangled_name);
    context->add_struct(signature);
}
