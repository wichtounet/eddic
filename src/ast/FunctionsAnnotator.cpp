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
        AUTO_RECURSE_SIMPLE_LOOPS()
        AUTO_RECURSE_FOREACH()
        AUTO_RECURSE_BRANCHES()
        AUTO_RECURSE_BUILTIN_OPERATORS()
        AUTO_RECURSE_COMPOSED_VALUES()
        AUTO_RECURSE_MEMBER_VALUE()
        AUTO_RECURSE_UNARY_VALUES()
        AUTO_RECURSE_VARIABLE_OPERATIONS()
        AUTO_RECURSE_STRUCT()
        AUTO_RECURSE_CONSTRUCTOR()
        AUTO_RECURSE_DESTRUCTOR()
        AUTO_RECURSE_SWITCH()
        AUTO_RECURSE_SWITCH_CASE()
        AUTO_RECURSE_DEFAULT_CASE()

        void operator()(ast::SourceFile& program){
            context = program.Content->context;

            visit_each(*this, program.Content->blocks);
        }

        void operator()(ast::FunctionDeclaration& declaration){
            currentFunction = context->getFunction(declaration.Content->mangledName);

            visit_each(*this, declaration.Content->instructions);
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

        void operator()(ast::FunctionCall& functionCall){
            if(functionCall.Content->template_types.empty() || functionCall.Content->resolved){
                visit_each(*this, functionCall.Content->values);

                std::string name = functionCall.Content->function_name;

                auto types = get_types(functionCall);

                std::string mangled = mangle(name, types);

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
                    throw SemanticalException("The function \"" + unmangle(mangled) + "\" does not exists", functionCall.Content->position);
                }
            }
        }

        void operator()(ast::MemberFunctionCall& functionCall){
            if(functionCall.Content->template_types.empty() || functionCall.Content->resolved){
                auto var = functionCall.Content->context->getVariable(functionCall.Content->object_name);
                auto type = var->type();
                auto struct_type = type->is_pointer() ? type->data_type() : type;

                visit_each(*this, functionCall.Content->values);

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

        void operator()(ast::Return& return_){
            return_.Content->function = currentFunction;

            visit(*this, return_.Content->value);
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
