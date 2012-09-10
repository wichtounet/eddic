//=======================================================================
// Copyright Baptiste Wicht 2011.
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

#include "ast/FunctionsAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"

using namespace eddic;

namespace {

class MemberFunctionAnnotator : public boost::static_visitor<> {
    private:
        std::shared_ptr<GlobalContext> context;
        std::string parent_struct;
        ast::Struct current_struct;

    public:
        void operator()(ast::SourceFile& program){
            context = program.Content->context;

            visit_each(*this, program.Content->blocks);
        }
        
        void operator()(ast::Struct& struct_){
            current_struct = struct_;
            parent_struct = struct_.Content->name;

            visit_each_non_variant(*this, struct_.Content->constructors);
            visit_each_non_variant(*this, struct_.Content->destructors);
            visit_each_non_variant(*this, struct_.Content->functions);

            parent_struct = "";
        }

        template<typename T>
        void annotate(T& declaration){
            if(!declaration.Content->marked){
                declaration.Content->struct_name = parent_struct;
                declaration.Content->struct_type = current_struct.Content->struct_type;
                
                ast::PointerType paramType;

                if(current_struct.Content->template_types.empty()){
                    ast::SimpleType struct_type;
                    struct_type.type = parent_struct;
                    struct_type.const_ = false;

                    paramType.type = struct_type;
                } else {
                    ast::TemplateType struct_type;
                    struct_type.type = parent_struct;
                    struct_type.template_types = current_struct.Content->template_types;
                    struct_type.resolved = true;

                    paramType.type = struct_type;
                }
                
                ast::FunctionParameter param;
                param.parameterName = "this";
                param.parameterType = paramType;

                declaration.Content->parameters.insert(declaration.Content->parameters.begin(), param);
            }
        }

        void operator()(ast::Constructor& constructor){
            annotate(constructor);
        }

        void operator()(ast::Destructor& destructor){
            annotate(destructor);
        }
         
        void operator()(ast::FunctionDeclaration& declaration){
            if(!parent_struct.empty()){
                annotate(declaration);
            }
        }

        AUTO_IGNORE_OTHERS()
};

class FunctionInserterVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<GlobalContext> context;

    public:
        AUTO_RECURSE_STRUCT()

        void operator()(ast::SourceFile& program){
            context = program.Content->context;

            visit_each(*this, program.Content->blocks);
        }
         
        void operator()(ast::FunctionDeclaration& declaration){
            if(!declaration.Content->marked){
                auto return_type = visit(ast::TypeTransformer(context), declaration.Content->returnType);
                auto signature = std::make_shared<Function>(return_type, declaration.Content->functionName);

                if(return_type->is_array()){
                    throw SemanticalException("Cannot return array from function", declaration.Content->position);
                }

                if(return_type->is_custom_type()){
                    throw SemanticalException("Cannot return struct from function", declaration.Content->position);
                }

                for(auto& param : declaration.Content->parameters){
                    auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
                    signature->parameters.push_back(ParameterType(param.parameterName, paramType));
                }

                signature->struct_ = declaration.Content->struct_name;
                signature->struct_type = declaration.Content->struct_type;
                signature->context = declaration.Content->context;

                declaration.Content->mangledName = signature->mangledName = mangle(signature);

                if(context->exists(signature->mangledName)){
                    throw SemanticalException("The function " + signature->mangledName + " has already been defined", declaration.Content->position);
                }

                context->addFunction(signature);
            }
        }

        void operator()(ast::Constructor& constructor){
            if(!constructor.Content->marked){
                auto signature = std::make_shared<Function>(VOID, "ctor");

                for(auto& param : constructor.Content->parameters){
                    auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
                    signature->parameters.push_back(ParameterType(param.parameterName, paramType));
                }

                signature->struct_ = constructor.Content->struct_name;
                signature->struct_type = constructor.Content->struct_type;

                constructor.Content->mangledName = signature->mangledName = mangle_ctor(signature);

                if(context->exists(signature->mangledName)){
                    throw SemanticalException("The constructor " + signature->name + " has already been defined", constructor.Content->position);
                }

                context->addFunction(signature);
                context->getFunction(signature->mangledName)->context = constructor.Content->context;
            }
        }

        void operator()(ast::Destructor& destructor){
            if(!destructor.Content->marked){
                auto signature = std::make_shared<Function>(VOID, "dtor");

                for(auto& param : destructor.Content->parameters){
                    auto paramType = visit(ast::TypeTransformer(context), param.parameterType);
                    signature->parameters.push_back(ParameterType(param.parameterName, paramType));
                }

                signature->struct_ = destructor.Content->struct_name;
                signature->struct_type = destructor.Content->struct_type;

                destructor.Content->mangledName = signature->mangledName = mangle_dtor(signature);

                if(context->exists(signature->mangledName)){
                    throw SemanticalException("Only one destructor per struct is allowed", destructor.Content->position);
                }

                context->addFunction(signature);
                context->getFunction(signature->mangledName)->context = destructor.Content->context;
            }
        }

        AUTO_IGNORE_OTHERS()

    private:
        std::string parent_struct;
};

class FunctionCheckerVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<Function> currentFunction;
        std::shared_ptr<GlobalContext> context;

    public:
        AUTO_RECURSE_GLOBAL_DECLARATION() 
        AUTO_RECURSE_MEMBER_VALUE()
        AUTO_RECURSE_STRUCT()

        void operator()(ast::Constructor& function){
            check_each(function.Content->instructions);
        }

        void operator()(ast::Destructor& function){
            check_each(function.Content->instructions);
        }

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

                if(functionCall.Content->template_types.empty() || functionCall.Content->resolved){
                    check_each(functionCall.Content->values);

                    //If the function has already been resolved, we can return directly
                    if(functionCall.Content->function){
                        return;
                    }

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
                                member_function_call.Content->resolved = functionCall.Content->resolved;
                                member_function_call.Content->template_types = functionCall.Content->template_types;
                                member_function_call.Content->values = functionCall.Content->values;

                                value = member_function_call;

                                return;
                            }
                        }

                        throw SemanticalException("The function \"" + unmangle(original_mangled) + "\" does not exists", functionCall.Content->position);
                    }
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

        void operator()(ast::FunctionDeclaration& declaration){
            currentFunction = context->getFunction(declaration.Content->mangledName);

            check_each(declaration.Content->instructions);
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
            ASSERT_PATH_NOT_TAKEN("Should be handled by check_value");
        }

        bool is_init(ast::Value& value){
            if(auto* ptr = boost::get<ast::VariableValue>(&value)){
                return ptr->Content->var != 0;
            }

            ASSERT_PATH_NOT_TAKEN("Unhandled value type");
        }

        void operator()(ast::MemberFunctionCall& functionCall){
            if(functionCall.Content->template_types.empty() || functionCall.Content->resolved){
                //It is possible that the object has not been handled by the template engine at this point
                if(!is_init(functionCall.Content->object)){
                    return;
                }

                check_value(functionCall.Content->object);
                check_each(functionCall.Content->values);

                //If the function has already been resolved, we can return directly
                if(functionCall.Content->function){
                    return;
                }

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

        void operator()(ast::Unary& value){
            check_value(value.Content->value);
        }

        void operator()(ast::BuiltinOperator& builtin){
            check_each(builtin.Content->values);
        }

        void operator()(ast::Expression& value){
            check_value(value.Content->first);

            for_each(value.Content->operations.begin(), value.Content->operations.end(), 
                [&](ast::Operation& operation){ check_value(operation.get<1>()); });
        }

        void operator()(ast::Return& return_){
            return_.Content->function = currentFunction;

            check_value(return_.Content->value);
        }

        AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

void ast::defineMemberFunctions(ast::SourceFile& program){
    MemberFunctionAnnotator annotator;
    annotator(program);
}

void ast::defineFunctions(ast::SourceFile& program){
    //First phase : Collect functions
    FunctionInserterVisitor inserterVisitor;
    inserterVisitor(program);

    //Second phase : Verify calls
    FunctionCheckerVisitor checkerVisitor;
    checkerVisitor(program);
}
