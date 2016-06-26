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

    if(struct_.is_template_instantation()){
        std::vector<std::shared_ptr<const eddic::Type>> template_types;

        ast::TypeTransformer transformer(context);

        for(auto& type : struct_.inst_template_types){
            template_types.push_back(visit(transformer, type));
        }

        struct_.struct_type = new_template_type(context, struct_.name, template_types);
    } else {
        struct_.struct_type = new_type(context, struct_.name, false);
    }

    //Annotate functions with the parent struct
    for(auto& block : struct_.blocks){
        if(auto* ptr = boost::get<ast::TemplateFunctionDeclaration>(&block)){
            if(!ptr->is_template()){
                auto& function = *ptr;

                if(function.context){
                    function.context->struct_type = struct_.struct_type;
                }
            }
        } else if(auto* ptr = boost::get<ast::Constructor>(&block)){
            auto& function = *ptr;

            if(function.context){
                function.context->struct_type = struct_.struct_type;
            }
        } else if(auto* ptr = boost::get<ast::Destructor>(&block)){
            auto& function = *ptr;

            if(function.context){
                function.context->struct_type = struct_.struct_type;
            }
        }
    }

    auto mangled_name = struct_.struct_type->mangle();

    if(context->struct_exists(mangled_name)){
        context->error_handler.semantical_exception("The structure " + mangled_name + " has already been defined", struct_);
    }

    auto signature = std::make_shared<eddic::Struct>(mangled_name);
    context->add_struct(signature);
}
