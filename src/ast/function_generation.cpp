//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "VisitorUtils.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "mangling.hpp"
#include "FunctionContext.hpp"

#include "ast/function_generation.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/Struct.hpp"

using namespace eddic;

void ast::FunctionGenerationPass::apply_struct(ast::Struct& struct_, bool indicator){
    if(indicator){
        return;
    }

    auto platform = context->target_platform();

    bool default_constructor = false;
    bool constructor = false;
    bool destructor = false;
    bool copy_constructor = false;

    for(auto& block : struct_.Content->blocks){
        if(boost::get<ast::Destructor>(&block)){
            destructor = true;
        } else if(auto* ptr = boost::get<ast::Constructor>(&block)){
            constructor = true;

            auto& c = *ptr;

            if(c.Content->parameters.size() == 0){
                default_constructor = true;
            } else if(c.Content->parameters.size() == 1){
                auto& parameter = c.Content->parameters.front();
                auto parameter_type = visit(ast::TypeTransformer(context), parameter.parameterType);
                
                if(parameter_type == new_pointer_type(struct_.Content->struct_type)){
                    copy_constructor = true;
                }
            }
        }
    }

    //Generate default constructor if necessary
    if(!default_constructor && !constructor){
        ast::Constructor c;
        c.Content->context = std::make_shared<FunctionContext>(context, context, platform, configuration);
        c.Content->struct_type = struct_.Content->struct_type;

        std::vector<std::shared_ptr<const eddic::Type>> types;
        c.Content->mangledName = mangle_ctor(types, c.Content->struct_type);
                
        struct_.Content->blocks.push_back(c);
    }

    //Generate destructor if necessary
    if(!destructor){
        ast::Destructor d;
        d.Content->context = std::make_shared<FunctionContext>(context, context, platform, configuration);
        d.Content->struct_type = struct_.Content->struct_type;
        d.Content->mangledName = mangle_dtor(d.Content->struct_type);
                
        struct_.Content->blocks.push_back(d);
    }

    //Generate copy constructor if necessary
    if(!copy_constructor){
        auto type = struct_.Content->struct_type;
        auto struct_type = context->get_struct(type->mangle());

        bool possible = true;
        for(auto& member : struct_type->members){
            if(member->type->is_array() || member->type->is_custom_type() || member->type->is_template_type()){
                possible = false;
                break;
            }
        }

        if(possible){
            auto function_context = std::make_shared<FunctionContext>(context, context, platform, configuration);

            function_context->addParameter("this", new_pointer_type(type));
            function_context->addParameter("rhs", new_pointer_type(type));

            ast::Constructor c;
            c.Content->context = function_context;
            c.Content->struct_type = type;

            std::vector<std::shared_ptr<const eddic::Type>> types;
            types.push_back(new_pointer_type(type));
            c.Content->mangledName = mangle_ctor(types, type);

            FunctionParameter parameter;
            parameter.parameterName = "rhs";

            ast::SimpleType type;
            type.const_ = false;
            type.type = c.Content->struct_type->type();

            ast::PointerType ptr_type;
            ptr_type.type = type;

            parameter.parameterType = ptr_type;

            c.Content->parameters.push_back(parameter);

            for(auto& member : struct_type->members){
                auto& name = member->name;

                ast::Assignment assignment;
                assignment.Content->context = function_context;

                ast::VariableValue this_value;
                this_value.Content->context = function_context;
                this_value.Content->variableName = "this";
                this_value.Content->var = function_context->getVariable("this");

                ast::Expression left_expression;
                left_expression.Content->context = function_context;
                left_expression.Content->first = this_value;
                left_expression.Content->operations.push_back(boost::make_tuple(ast::Operator::DOT, name));

                ast::VariableValue rhs_value;
                rhs_value.Content->context = function_context;
                rhs_value.Content->variableName = "rhs";
                rhs_value.Content->var = function_context->getVariable("rhs");

                ast::Expression right_expression;
                right_expression.Content->context = function_context;
                right_expression.Content->first = rhs_value;
                right_expression.Content->operations.push_back(boost::make_tuple(ast::Operator::DOT, name));

                assignment.Content->left_value = left_expression;
                assignment.Content->value = right_expression;

                c.Content->instructions.push_back(std::move(assignment));
            }

            struct_.Content->blocks.push_back(std::move(c));
        }
    }
}
