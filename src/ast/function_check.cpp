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
#include "ast/IsConstantVisitor.hpp"
#include "ast/GetConstantValue.hpp"

using namespace eddic;

namespace {

class FunctionCheckerVisitor : public boost::static_visitor<> {
    public:
        std::shared_ptr<GlobalContext> context;
        std::shared_ptr<Function> currentFunction;
        std::shared_ptr<ast::TemplateEngine> template_engine;

        FunctionCheckerVisitor(std::shared_ptr<ast::TemplateEngine> template_engine) : template_engine(template_engine) {}

        void operator()(ast::DefaultCase& default_case){
            check_each(default_case.instructions);
        }

        void operator()(ast::Foreach& foreach){
            template_engine->check_type(foreach.Content->variableType, foreach.Content->position);

            if(check_variable(foreach.Content->context, foreach.Content->variableName, foreach.Content->position)){
                auto type = visit(ast::TypeTransformer(context), foreach.Content->variableType);

                auto var = foreach.Content->context->addVariable(foreach.Content->variableName, type);
                var->set_source_position(foreach.Content->position);
            }

            check_each(foreach.Content->instructions);
        }

        void operator()(ast::ForeachIn& foreach){
            template_engine->check_type(foreach.Content->variableType, foreach.Content->position);

            if(check_variable(foreach.Content->context, foreach.Content->variableName, foreach.Content->position)){
                if(!foreach.Content->context->exists(foreach.Content->arrayName)){
                    throw SemanticalException("The foreach array " + foreach.Content->arrayName  + " has not been declared", foreach.Content->position);
                }

                auto type = visit(ast::TypeTransformer(context), foreach.Content->variableType);

                foreach.Content->var = foreach.Content->context->addVariable(foreach.Content->variableName, type);
                foreach.Content->var->set_source_position(foreach.Content->position);

                foreach.Content->arrayVar = foreach.Content->context->getVariable(foreach.Content->arrayName);
                foreach.Content->iterVar = foreach.Content->context->generate_variable("foreach_iter", INT);

                //Add references to variables
                foreach.Content->var->add_reference();
                foreach.Content->iterVar->add_reference();
                foreach.Content->arrayVar->add_reference();
            }

            check_each(foreach.Content->instructions);
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

        ast::VariableValue this_variable(std::shared_ptr<Context> context, ast::Position position){
            ast::VariableValue variable_value;

            variable_value.Content->context = context;
            variable_value.Content->position = position;
            variable_value.Content->variableName = "this";
            variable_value.Content->var = context->getVariable("this");
            
            return variable_value;
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

                if(context->exists(mangled)){
                    context->addReference(mangled);

                    functionCall.Content->mangled_name = mangled;
                    functionCall.Content->function = context->getFunction(mangled);
                } else {
                    auto local_context = functionCall.Content->context->function();

                    if(local_context && local_context->struct_type && context->struct_exists(local_context->struct_type->mangle())){
                        auto struct_type = local_context->struct_type;

                        do {
                            mangled = mangle(name, types, struct_type);

                            if(context->exists(mangled)){
                                context->addReference(mangled);

                                ast::Cast cast_value;
                                cast_value.Content->resolved_type = new_pointer_type(struct_type);
                                cast_value.Content->value = this_variable(functionCall.Content->context, functionCall.Content->position);

                                ast::CallOperationValue function_call_operation;
                                function_call_operation.function_name = functionCall.Content->function_name; 
                                function_call_operation.template_types = functionCall.Content->template_types; 
                                function_call_operation.values = functionCall.Content->values;
                                function_call_operation.function = context->getFunction(mangled);

                                ast::Expression member_function_call;
                                member_function_call.Content->context = functionCall.Content->context;
                                member_function_call.Content->position = functionCall.Content->position;
                                member_function_call.Content->first = cast_value;
                                member_function_call.Content->operations.push_back(boost::make_tuple(ast::Operator::CALL, function_call_operation));

                                value = member_function_call;

                                check_value(value);

                                return;
                            }

                            struct_type = context->get_struct(struct_type)->parent_type;
                        } while(struct_type);
                    }

                    throw SemanticalException("The function \"" + unmangle(original_mangled) + "\" does not exists", functionCall.Content->position);
                }
            } else if(auto* ptr = boost::get<ast::VariableValue>(&value)){
                auto& variable = *ptr;
                if (!variable.Content->context->exists(variable.Content->variableName)) {
                    auto context = variable.Content->context->function();
                    auto global_context = variable.Content->context->global();

                    if(context && context->struct_type && global_context->struct_exists(context->struct_type->mangle())){
                        auto struct_type = global_context->get_struct(context->struct_type->mangle());

                        if(struct_type->member_exists(variable.Content->variableName)){
                            ast::Expression member_value;
                            member_value.Content->context = variable.Content->context;
                            member_value.Content->position = variable.Content->position;
                            member_value.Content->first = this_variable(variable.Content->context, variable.Content->position);
                            member_value.Content->operations.push_back(boost::make_tuple(ast::Operator::DOT, variable.Content->variableName));

                            value = member_value;

                            check_value(value);

                            return;
                        }
                    }

                    throw SemanticalException("Variable " + variable.Content->variableName + " has not been declared", variable.Content->position);
                }

                //Reference the variable
                variable.Content->var = variable.Content->context->getVariable(variable.Content->variableName);
                variable.Content->var->add_reference();
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

        std::vector<std::shared_ptr<const Type>> get_types(std::vector<ast::Value>& values){
            std::vector<std::shared_ptr<const Type>> types;

            ast::GetTypeVisitor visitor;
            for(auto& value : values){
                types.push_back(visit(visitor, value));
            }

            return types;
        }
    
        template<typename T>
        std::vector<std::shared_ptr<const Type>> get_types(T& functionCall){
            return get_types(functionCall.Content->values);
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
            check_value(assignment.Content->left_value);
            check_value(assignment.Content->value);
        }

        void operator()(ast::VariableDeclaration& declaration){
            template_engine->check_type(declaration.Content->variableType, declaration.Content->position);

            if(declaration.Content->value){
                check_value(*declaration.Content->value);
            }

            if(check_variable(declaration.Content->context, declaration.Content->variableName, declaration.Content->position)){
                auto type = visit(ast::TypeTransformer(context), declaration.Content->variableType);

                //If it's a standard type
                if(type->is_standard_type()){
                    if(type->is_const()){
                        if(!declaration.Content->value){
                            throw SemanticalException("A constant variable must have a value", declaration.Content->position);
                        }

                        if(!visit(ast::IsConstantVisitor(), *declaration.Content->value)){
                            throw SemanticalException("The value must be constant", declaration.Content->position);
                        }

                        auto var = declaration.Content->context->addVariable(declaration.Content->variableName, type, *declaration.Content->value);
                        var->set_source_position(declaration.Content->position);
                    } else {
                        auto var = declaration.Content->context->addVariable(declaration.Content->variableName, type);
                        var->set_source_position(declaration.Content->position);
                    }
                } 
                //If it's a pointer type
                else if(type->is_pointer()){
                    if(type->is_const()){
                        throw SemanticalException("Pointer types cannot be const", declaration.Content->position);
                    }

                    auto var = declaration.Content->context->addVariable(declaration.Content->variableName, type);
                    var->set_source_position(declaration.Content->position);
                } 
                //If it's a array
                else if(type->is_array()){
                    auto var = declaration.Content->context->addVariable(declaration.Content->variableName, type);
                    var->set_source_position(declaration.Content->position);
                } 
                //If it's a template or custom type
                else {
                    auto mangled = type->mangle();

                    if(context->struct_exists(mangled)){
                        if(type->is_const()){
                            throw SemanticalException("Custom types cannot be const", declaration.Content->position);
                        }

                        auto var = declaration.Content->context->addVariable(declaration.Content->variableName, type);
                        var->set_source_position(declaration.Content->position);
                    } else {
                        throw SemanticalException("The type \"" + mangled + "\" does not exists", declaration.Content->position);
                    }
                }
            }

            if(declaration.Content->value){
                check_value(*declaration.Content->value);
            }
        }

        void operator()(ast::Delete& delete_){
            if (!delete_.Content->context->exists(delete_.Content->variable_name)) {
                throw SemanticalException("Variable " + delete_.Content->variable_name + " has not been declared", delete_.Content->position);
            }

            delete_.Content->variable = delete_.Content->context->getVariable(delete_.Content->variable_name);
            delete_.Content->variable->add_reference();
        }
    
        void operator()(ast::StructDeclaration& declaration){
            template_engine->check_type(declaration.Content->variableType, declaration.Content->position);

            check_each(declaration.Content->values);

            if(check_variable(declaration.Content->context, declaration.Content->variableName, declaration.Content->position)){
                auto type = visit(ast::TypeTransformer(context), declaration.Content->variableType);

                if(!type->is_custom_type() && !type->is_template_type()){
                    throw SemanticalException("Only custom types take parameters when declared", declaration.Content->position);
                }

                auto mangled = type->mangle();

                if(context->struct_exists(mangled)){
                    if(type->is_const()){
                        throw SemanticalException("Custom types cannot be const", declaration.Content->position);
                    }

                    auto var = declaration.Content->context->addVariable(declaration.Content->variableName, type);
                    var->set_source_position(declaration.Content->position);
                } else {
                    throw SemanticalException("The type \"" + mangled + "\" does not exists", declaration.Content->position);
                }
            }
        }

        template<typename ArrayDeclaration>
        void declare_array(ArrayDeclaration& declaration){
            template_engine->check_type(declaration.Content->arrayType, declaration.Content->position);

            check_value(declaration.Content->size);

            if(check_variable(declaration.Content->context, declaration.Content->arrayName, declaration.Content->position)){
                auto element_type = visit(ast::TypeTransformer(context), declaration.Content->arrayType);

                if(element_type->is_array()){
                    throw SemanticalException("Arrays of arrays are not supported", declaration.Content->position);
                }

                auto constant = visit(ast::IsConstantVisitor(), declaration.Content->size);

                if(!constant){
                    throw SemanticalException("Array size must be constant", declaration.Content->position);
                }

                auto value = visit(ast::GetConstantValue(), declaration.Content->size);
                auto size = boost::get<int>(value);

                auto var = declaration.Content->context->addVariable(declaration.Content->arrayName, new_array_type(element_type, size));
                var->set_source_position(declaration.Content->position);
            }
        }

        void operator()(ast::GlobalArrayDeclaration& declaration){
            declare_array(declaration);
        }

        void operator()(ast::ArrayDeclaration& declaration){
            declare_array(declaration);
        }
        
        void operator()(ast::PrefixOperation& operation){
            check_value(operation.Content->left_value);
        }

        void operator()(ast::Return& return_){
            return_.Content->function = currentFunction;

            check_value(return_.Content->value);
        }
    
        void operator()(ast::Ternary& ternary){
            check_value(ternary.Content->condition);
            check_value(ternary.Content->true_value);
            check_value(ternary.Content->false_value);
        }

        void operator()(ast::BuiltinOperator& builtin){
            check_each(builtin.Content->values);
        }

        void operator()(ast::Cast& cast){
            check_value(cast.Content->value);
        }
        
        void operator()(ast::Expression& value){
            check_value(value.Content->first);

            auto context = value.Content->context->global();
            
            auto type = visit(ast::GetTypeVisitor(), value.Content->first);
            for(auto& op : value.Content->operations){
                if(op.get<1>()){
                    if(auto* ptr = boost::get<ast::Value>(&*op.get<1>())){
                        check_value(*ptr);
                    } else if(auto* ptr = boost::get<ast::CallOperationValue>(&*op.get<1>())){
                        check_each(ptr->values);
                    }
                }

                template_engine->check_member_function(type, op, value.Content->position);

                if(op.get<0>() == ast::Operator::DOT){
                    auto struct_type = value.Content->context->global()->get_struct(type);

                    //We delay it
                    if(!struct_type){
                        return;
                    }

                    //Reference the structure
                    struct_type->add_reference();

                    auto member = boost::get<std::string>(*op.get<1>());

                    if(!struct_type->member_exists(member)){ 
                        throw SemanticalException("The struct " + struct_type->name + " has no member named " + member, value.Content->position);
                    }

                    //Add a reference to the member
                    (*struct_type)[member]->add_reference();
                }

                if(op.get<0>() == ast::Operator::CALL){
                    auto struct_type = type->is_pointer() ? type->data_type() : type;

                    auto& call_value = boost::get<ast::CallOperationValue>(*op.get<1>());
                    std::string name = call_value.function_name;

                    auto types = get_types(call_value.values);

                    bool found = false;
                    std::string mangled;
                    
                    do {
                        mangled = mangle(name, types, struct_type);

                        if(context->exists(mangled)){
                            context->addReference(mangled);

                            call_value.mangled_name = mangled;
                            call_value.function = context->getFunction(mangled);
                            found = true;
                            break;
                        }

                        struct_type = context->get_struct(struct_type)->parent_type;
                    } while(struct_type);

                    if(!found){
                        throw SemanticalException("The member function \"" + unmangle(mangled) + "\" does not exists", value.Content->position);
                    }
                }

                type = ast::operation_type(type, value.Content->context, op);
            }
        }

        void operator()(ast::New& new_){
            check_each(new_.Content->values);
        }
    
        void operator()(ast::Swap& swap){
            if (swap.Content->lhs == swap.Content->rhs) {
                throw SemanticalException("Cannot swap a variable with itself", swap.Content->position);
            }

            if (!swap.Content->context->exists(swap.Content->lhs) || !swap.Content->context->exists(swap.Content->rhs)) {
                throw SemanticalException("Variable has not been declared in the swap", swap.Content->position);
            }

            swap.Content->lhs_var = swap.Content->context->getVariable(swap.Content->lhs);
            swap.Content->rhs_var = swap.Content->context->getVariable(swap.Content->rhs);

            //Reference both variables
            swap.Content->lhs_var->add_reference();
            swap.Content->rhs_var->add_reference();
        }

        bool check_variable(std::shared_ptr<Context> context, const std::string& name, const ast::Position& position){
            if(context->exists(name)){
                auto var = context->getVariable(name);

                if(var->source_position() == position){
                    return false; 
                } else {
                    throw SemanticalException("The Variable " + name + " has already been declared", position);
                }
            }

            return true;
        }

        void operator()(ast::GlobalVariableDeclaration& declaration){
            template_engine->check_type(declaration.Content->variableType, declaration.Content->position);

            if(check_variable(declaration.Content->context, declaration.Content->variableName, declaration.Content->position)){
                if(!visit(ast::IsConstantVisitor(), *declaration.Content->value)){
                    throw SemanticalException("The value must be constant", declaration.Content->position);
                }

                auto type = visit(ast::TypeTransformer(context), declaration.Content->variableType);

                auto var = declaration.Content->context->addVariable(declaration.Content->variableName, type, *declaration.Content->value);
                var->set_source_position(declaration.Content->position);
            }
        }

        bool is_valid(const ast::Type& type){
            if(auto* ptr = boost::get<ast::ArrayType>(&type)){
                return is_valid(ptr->type.get());
            } else if(auto* ptr = boost::get<ast::SimpleType>(&type)){
                if(is_standard_type(ptr->type)){
                    return true;
                }

                auto t = visit_non_variant(ast::TypeTransformer(context), *ptr);
                return context->struct_exists(t->mangle());
            } else if(auto* ptr = boost::get<ast::PointerType>(&type)){
                return is_valid(ptr->type.get());
            } else if(auto* ptr = boost::get<ast::TemplateType>(&type)){
                auto t = visit_non_variant(ast::TypeTransformer(context), *ptr);
                return context->struct_exists(t->mangle());
            }

            eddic_unreachable("Invalid type");
        }

        template<typename Function>
        void visit_function(Function& declaration){
            //Add all the parameters to the function context
            for(auto& parameter : declaration.Content->parameters){
                template_engine->check_type(parameter.parameterType, declaration.Content->position);

                if(check_variable(declaration.Content->context, parameter.parameterName, declaration.Content->position)){
                    if(!is_valid(parameter.parameterType)){
                        throw SemanticalException("Invalid parameter type " + ast::to_string(parameter.parameterType), declaration.Content->position);
                    }

                    auto type = visit(ast::TypeTransformer(context), parameter.parameterType);
                    auto var = declaration.Content->context->addParameter(parameter.parameterName, type);    
                    var->set_source_position(declaration.Content->position);
                }
            }

            check_each(declaration.Content->instructions);
        }

        void operator()(ast::FunctionCall&){
            eddic_unreachable("Should be handled by check_value");
        }

        void operator()(ast::VariableValue&){
            eddic_unreachable("Should be handled by check_value");
        }

        AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace
    
void ast::FunctionCheckPass::apply_struct(ast::Struct& struct_, bool indicator){
    if(!indicator && context->is_recursively_nested(context->get_struct(struct_.Content->struct_type))){
        throw SemanticalException("The structure " + struct_.Content->struct_type->mangle() + " is invalidly nested", struct_.Content->position);
    }
}
    
void ast::FunctionCheckPass::apply_function(ast::FunctionDeclaration& declaration){
    FunctionCheckerVisitor visitor(template_engine);
    visitor.context = context;
    visitor.currentFunction = context->getFunction(declaration.Content->mangledName);
    visitor.visit_function(declaration);

    auto return_type = visit(ast::TypeTransformer(context), declaration.Content->returnType);
    if(return_type->is_custom_type() || return_type->is_template_type()){
        declaration.Content->context->addParameter("__ret", new_pointer_type(return_type));
    }
}

void ast::FunctionCheckPass::apply_struct_function(ast::FunctionDeclaration& declaration){
    apply_function(declaration);
}

void ast::FunctionCheckPass::apply_struct_constructor(ast::Constructor& constructor){
    FunctionCheckerVisitor visitor(template_engine);
    visitor.context = context;
    visitor.visit_function(constructor);
}

void ast::FunctionCheckPass::apply_struct_destructor(ast::Destructor& destructor){
    FunctionCheckerVisitor visitor(template_engine);
    visitor.context = context;
    visitor.visit_function(destructor);
}

void ast::FunctionCheckPass::apply_program(ast::SourceFile& program, bool indicator){
    context = program.Content->context;

    if(!indicator){
        FunctionCheckerVisitor visitor(template_engine);
        visitor.context = context;

        for(auto& block : program.Content->blocks){
            if(auto* ptr = boost::get<ast::GlobalArrayDeclaration>(&block)){
                visit_non_variant(visitor, *ptr);
            } else if(auto* ptr = boost::get<ast::GlobalVariableDeclaration>(&block)){
                visit_non_variant(visitor, *ptr);
            }
        }
    }
}
