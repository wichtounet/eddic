//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "SemanticalException.hpp"
#include "VisitorUtils.hpp"
#include "mangling.hpp"
#include "Options.hpp"
#include "Type.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"

#include "ast/function_check.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/TemplateEngine.hpp"

using namespace eddic;

namespace {

class FunctionCheckerVisitor : public boost::static_visitor<> {
    public:
        std::shared_ptr<GlobalContext> context;
        std::shared_ptr<Function> currentFunction;
        std::shared_ptr<ast::TemplateEngine> template_engine;

        FunctionCheckerVisitor(std::shared_ptr<ast::TemplateEngine> template_engine) : template_engine(template_engine) {}

        AUTO_RECURSE_GLOBAL_DECLARATION() 

        void operator()(ast::DefaultCase& default_case){
            check_each(default_case.instructions);
        }

        void operator()(ast::Foreach& foreach_){
            check_each(foreach_.Content->instructions);
        }

        void operator()(ast::ForeachIn& foreach_){
            check_each(foreach_.Content->instructions);
        }

        void operator()(ast::If& if_){
            check_value(if_.Content->condition);
            check_each(if_.Content->instructions);
            visit_each_non_variant(*this, if_.Content->elseIfs);
            visit_optional_non_variant(*this, if_.Content->else_);
        }

        void operator()(ast::ElseIf& elseIf){
            check_value(elseIf.condition);
            check_each(elseIf.instructions);
        }
        
        void operator()(ast::Else& else_){
            check_each(else_.instructions);
        }

        template<typename T>
        void check_each(std::vector<T>& values){
            for(std::size_t i = 0; i < values.size(); ++i){
                check_value(values[i]);
            }
        }

        template<typename V>
        void check_value(V& value){
            if(auto* ptr = boost::get<ast::FunctionCall>(&value)){
                auto functionCall = *ptr;

                template_engine->check_function(functionCall);

                check_each(functionCall.Content->values);

                std::string name = functionCall.Content->function_name;

                auto types = get_types(functionCall);

                auto mangled = mangle(name, types);
                auto original_mangled = mangled;

                //If the function does not exists, try implicit conversions to pointers
                if(!context->exists(mangled)){
                    auto perms = permutations(types);

                    for(auto& perm : perms){
                        mangled = mangle(name, perm);

                        if(context->exists(mangled)){
                            break;
                        }
                    }
                }

                if(context->exists(mangled)){
                    context->addReference(mangled);

                    functionCall.Content->mangled_name = mangled;
                    functionCall.Content->function = context->getFunction(mangled);
                } else {
                    auto local_context = functionCall.Content->context->function();

                    if(local_context && local_context->struct_type && context->struct_exists(local_context->struct_type->mangle())){
                        auto struct_type = local_context->struct_type;

                        mangled = mangle(name, types, struct_type);

                        //If the function does not exists, try implicit conversions to pointers
                        if(!context->exists(mangled)){
                            auto perms = permutations(types);

                            for(auto& perm : perms){
                                mangled = mangle(name, perm, struct_type);

                                if(context->exists(mangled)){
                                    break;
                                }
                            }
                        }

                        if(context->exists(mangled)){
                            context->addReference(mangled);

                            ast::VariableValue variable_value;
                            variable_value.Content->context = functionCall.Content->context;
                            variable_value.Content->position = functionCall.Content->position;
                            variable_value.Content->variableName = "this";
                            variable_value.Content->var = functionCall.Content->context->getVariable("this");

                            ast::MemberFunctionCall member_function_call;
                            member_function_call.Content->function = context->getFunction(mangled);
                            member_function_call.Content->mangled_name = mangled;
                            member_function_call.Content->position = functionCall.Content->position;
                            member_function_call.Content->object = variable_value;
                            member_function_call.Content->function_name = functionCall.Content->function_name;
                            member_function_call.Content->template_types = functionCall.Content->template_types;
                            member_function_call.Content->values = functionCall.Content->values;

                            value = member_function_call;

                            return;
                        }
                    }

                    throw SemanticalException("The function \"" + unmangle(original_mangled) + "\" does not exists", functionCall.Content->position);
                }
            } else {
                visit(*this, value);
            }
        }

        void operator()(ast::For& for_){
            if(for_.Content->start){
                check_value(*for_.Content->start);
            }
            
            if(for_.Content->condition){
                check_value(*for_.Content->condition);
            }
            
            if(for_.Content->repeat){
                check_value(*for_.Content->repeat);
            }

            check_each(for_.Content->instructions);
        }
        
        void operator()(ast::While& while_){
            check_value(while_.Content->condition);
            check_each(while_.Content->instructions);
        }
        
        void operator()(ast::DoWhile& while_){
            check_value(while_.Content->condition);
            check_each(while_.Content->instructions);
        }

        void operator()(ast::SourceFile& program){
            context = program.Content->context;

            visit_each(*this, program.Content->blocks);
        }

        void permute(std::vector<std::vector<std::shared_ptr<const Type>>>& perms, std::vector<std::shared_ptr<const Type>>& types, int start){
            for(std::size_t i = start; i < types.size(); ++i){
                if(!types[i]->is_pointer() && !types[i]->is_array()){
                    std::vector<std::shared_ptr<const Type>> copy = types;

                    copy[i] = new_pointer_type(types[i]);

                    perms.push_back(copy);

                    permute(perms, copy, i + 1);
                }
            }
        }

        std::vector<std::vector<std::shared_ptr<const Type>>> permutations(std::vector<std::shared_ptr<const Type>>& types){
            std::vector<std::vector<std::shared_ptr<const Type>>> perms;

            permute(perms, types, 0);

            return perms;
        }
    
        template<typename T>
        std::vector<std::shared_ptr<const Type>> get_types(T& functionCall){
            std::vector<std::shared_ptr<const Type>> types;

            ast::GetTypeVisitor visitor;
            for(auto& value : functionCall.Content->values){
                types.push_back(visit(visitor, value));
            }

            return types;
        }

        void operator()(ast::FunctionCall&){
            eddic_unreachable("Should be handled by check_value");
        }

        void operator()(ast::MemberFunctionCall& functionCall){
            template_engine->check_member_function(functionCall);

            check_value(functionCall.Content->object);
            check_each(functionCall.Content->values);

            auto type = visit(ast::GetTypeVisitor(), functionCall.Content->object);
            auto struct_type = type->is_pointer() ? type->data_type() : type;

            std::string name = functionCall.Content->function_name;

            auto types = get_types(functionCall);

            std::string mangled = mangle(name, types, struct_type);

            //If the function does not exists, try implicit conversions to pointers
            if(!context->exists(mangled)){
                auto perms = permutations(types);

                for(auto& perm : perms){
                    mangled = mangle(name, perm, struct_type);

                    if(context->exists(mangled)){
                        break;
                    }
                }
            }

            if(context->exists(mangled)){
                context->addReference(mangled);

                functionCall.Content->mangled_name = mangled;
                functionCall.Content->function = context->getFunction(mangled);
            } else {
                throw SemanticalException("The member function \"" + unmangle(mangled) + "\" does not exists", functionCall.Content->position);
            }
        }
        
        void operator()(ast::Switch& switch_){
            check_value(switch_.Content->value);
            visit_each_non_variant(*this, switch_.Content->cases);
            visit_optional_non_variant(*this, switch_.Content->default_case);
        }

        void operator()(ast::SwitchCase& switch_case){
            check_value(switch_case.value);
            check_each(switch_case.instructions);
        }

        void operator()(ast::Assignment& assignment){
            visit(*this, assignment.Content->left_value);
            check_value(assignment.Content->value);
        }

        void operator()(ast::VariableDeclaration& declaration){
            if(declaration.Content->value){
                check_value(*declaration.Content->value);
            }
        }
        
        void operator()(ast::PrefixOperation& operation){
            check_value(operation.Content->left_value);
        }

        void operator()(ast::BuiltinOperator& builtin){
            check_each(builtin.Content->values);
        }

        void operator()(ast::Expression& value){
            check_value(value.Content->first);

            for(auto& op : value.Content->operations){
                if(op.get<1>()){
                    if(auto* ptr = boost::get<ast::Value>(&*op.get<1>())){
                        check_value(*ptr);
                    } else if(auto* ptr = boost::get<ast::CallOperationValue>(&*op.get<1>())){
                        check_each(ptr->get<2>());
                    }
                }
            }
        }

        void operator()(ast::Return& return_){
            return_.Content->function = currentFunction;

            check_value(return_.Content->value);
        }

        AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace
    
void ast::FunctionCheckPass::apply_function(ast::FunctionDeclaration& declaration){
    FunctionCheckerVisitor visitor(template_engine);
    visitor.context = context;
    visitor.currentFunction = context->getFunction(declaration.Content->mangledName);
    visitor.check_each(declaration.Content->instructions);
}

void ast::FunctionCheckPass::apply_struct_function(ast::FunctionDeclaration& function){
    apply_function(function);
}

void ast::FunctionCheckPass::apply_struct_constructor(ast::Constructor& constructor){
    FunctionCheckerVisitor visitor(template_engine);
    visitor.context = context;
    visitor.check_each(constructor.Content->instructions);
}

void ast::FunctionCheckPass::apply_struct_destructor(ast::Destructor& destructor){
    FunctionCheckerVisitor visitor(template_engine);
    visitor.context = context;
    visitor.check_each(destructor.Content->instructions);
}
