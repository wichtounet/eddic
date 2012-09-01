//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>
#include <memory>

#include "variant.hpp"
#include "SemanticalException.hpp"
#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "mangling.hpp"

#include "ast/VariablesAnnotator.hpp"
#include "ast/SourceFile.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/IsConstantVisitor.hpp"
#include "ast/GetConstantValue.hpp"
#include "ast/ASTVisitor.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/VariableType.hpp"

using namespace eddic;

namespace {

template<typename Visitor>
struct VisitorProxy : public boost::static_visitor<> {
    bool delayed = false;
    Visitor visitor;

    template<typename... Args>
    VisitorProxy(Args... args) : visitor(args...) {}

    AUTO_RECURSE_PROGRAM()
    AUTO_RECURSE_FUNCTION_CALLS()
    AUTO_RECURSE_SIMPLE_LOOPS()
    AUTO_RECURSE_BRANCHES()
    AUTO_RECURSE_BINARY_CONDITION()
    AUTO_RECURSE_TERNARY()
    AUTO_RECURSE_BUILTIN_OPERATORS()
    AUTO_RECURSE_UNARY_VALUES()
    AUTO_RECURSE_CAST_VALUES()
    AUTO_RECURSE_RETURN_VALUES()
    AUTO_RECURSE_SWITCH()
    AUTO_RECURSE_SWITCH_CASE()
    AUTO_RECURSE_DEFAULT_CASE()
    AUTO_RECURSE_NEW()
    AUTO_RECURSE_PREFIX()
    AUTO_RECURSE_SUFFIX()

    AUTO_IGNORE_TEMPLATE_FUNCTION_DECLARATION()
    AUTO_IGNORE_TEMPLATE_STRUCT()
    AUTO_IGNORE_FALSE()
    AUTO_IGNORE_TRUE()
    AUTO_IGNORE_NULL()
    AUTO_IGNORE_LITERAL()
    AUTO_IGNORE_CHAR_LITERAL()
    AUTO_IGNORE_FLOAT()
    AUTO_IGNORE_INTEGER()
    AUTO_IGNORE_INTEGER_SUFFIX()
    AUTO_IGNORE_IMPORT()
    AUTO_IGNORE_STANDARD_IMPORT()

    template<typename T>
    void operator()(T& t){
        if(!delayed){
            visit_non_variant(visitor, t);
        }
    }
};

struct VariablesVisitor : public boost::static_visitor<> {
    VisitorProxy<VariablesVisitor>* proxy = nullptr;
    std::shared_ptr<GlobalContext> context;

    VariablesVisitor(std::shared_ptr<GlobalContext> context) : context(context) {}
    
    void operator()(ast::Struct& struct_){
        if(context->is_recursively_nested(struct_.Content->struct_type->mangle())){
            throw SemanticalException("The structure " + struct_.Content->struct_type->mangle() + " is invalidly nested", struct_.Content->position);
        }

        visit_each_non_variant(*proxy, struct_.Content->constructors);
        visit_each_non_variant(*proxy, struct_.Content->destructors);
        visit_each_non_variant(*proxy, struct_.Content->functions);
    }

    bool is_resolved(const ast::Type& type){
        if(auto* ptr = boost::get<ast::TemplateType>(&type)){
            return ptr->resolved;
        }
        
        return true;
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

        ASSERT_PATH_NOT_TAKEN("Invalid type");
    }

    template<typename Function>
    void visit_function(Function& declaration){
        //Add all the parameters to the function context
        for(auto& parameter : declaration.Content->parameters){
            //if its not resolved, delay the resolution
            if(!is_resolved(parameter.parameterType)){
                proxy->delayed = true;
                return;
            }

            if(check_variable(declaration.Content->context, parameter.parameterName, declaration.Content->position)){
                if(!is_valid(parameter.parameterType)){
                    throw SemanticalException("Invalid parameter type " + ast::to_string(parameter.parameterType), declaration.Content->position);
                }

                auto type = visit(ast::TypeTransformer(context), parameter.parameterType);
                auto var = declaration.Content->context->addParameter(parameter.parameterName, type);    
                var->set_source_position(declaration.Content->position);
            }
        }

        visit_each(*proxy, declaration.Content->instructions);
    }
   
    void operator()(ast::FunctionDeclaration& declaration){
        visit_function(declaration);

        proxy->delayed = false;
    }

    void operator()(ast::Constructor& constructor){
        visit_function(constructor);
    }

    void operator()(ast::Destructor& destructor){
        visit_function(destructor);
    }

    void operator()(ast::MemberFunctionCall& functionCall){
        if (!functionCall.Content->context->exists(functionCall.Content->object_name)){
            throw SemanticalException("The variable " + functionCall.Content->object_name + " does not exists", functionCall.Content->position);
        }
        
        auto variable = functionCall.Content->context->getVariable(functionCall.Content->object_name);
        variable->addReference();

        visit_each(*proxy, functionCall.Content->values);
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
        //Delay if necessary
        if(!is_resolved(declaration.Content->variableType)){
            proxy->delayed = true;
            return;
        }

        if(check_variable(declaration.Content->context, declaration.Content->variableName, declaration.Content->position)){
            if(!visit(ast::IsConstantVisitor(), *declaration.Content->value)){
                throw SemanticalException("The value must be constant", declaration.Content->position);
            }

            auto type = visit(ast::TypeTransformer(context), declaration.Content->variableType);
            
            auto var = declaration.Content->context->addVariable(declaration.Content->variableName, type, *declaration.Content->value);
            var->set_source_position(declaration.Content->position);
        }
    }

    template<typename ArrayDeclaration>
    void declare_array(ArrayDeclaration& declaration){
        //Delay if necessary
        if(!is_resolved(declaration.Content->arrayType)){
            proxy->delayed = true;
            return;
        }

        visit(*proxy, declaration.Content->size);

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
    
    void operator()(ast::Foreach& foreach){
        //Delay if necessary
        if(!is_resolved(foreach.Content->variableType)){
            proxy->delayed = true;
            return;
        }

        if(check_variable(foreach.Content->context, foreach.Content->variableName, foreach.Content->position)){
            auto type = visit(ast::TypeTransformer(context), foreach.Content->variableType);

            auto var = foreach.Content->context->addVariable(foreach.Content->variableName, type);
            var->set_source_position(foreach.Content->position);
        }

        visit_each(*proxy, foreach.Content->instructions);
    }
    
    void operator()(ast::ForeachIn& foreach){
        //Delay if necessary
        if(!is_resolved(foreach.Content->variableType)){
            proxy->delayed = true;
            return;
        }

        if(check_variable(foreach.Content->context, foreach.Content->variableName, foreach.Content->position)){
            if(!foreach.Content->context->exists(foreach.Content->arrayName)){
                throw SemanticalException("The foreach array " + foreach.Content->arrayName  + " has not been declared", foreach.Content->position);
            }

            static int generated = 0;
            
            auto type = visit(ast::TypeTransformer(context), foreach.Content->variableType);

            foreach.Content->var = foreach.Content->context->addVariable(foreach.Content->variableName, type);
            foreach.Content->var->set_source_position(foreach.Content->position);

            foreach.Content->arrayVar = foreach.Content->context->getVariable(foreach.Content->arrayName);
            foreach.Content->iterVar = foreach.Content->context->addVariable("foreach_iter_" + toString(++generated), INT);
        }

        visit_each(*proxy, foreach.Content->instructions);
    }

    void operator()(ast::Assignment& assignment){
        visit(*proxy, assignment.Content->left_value);
        visit(*proxy, assignment.Content->value);
    }

    void operator()(ast::Delete& delete_){
        if (!delete_.Content->context->exists(delete_.Content->variable_name)) {
            throw SemanticalException("Variable " + delete_.Content->variable_name + " has not been declared", delete_.Content->position);
        }
        
        delete_.Content->variable = delete_.Content->context->getVariable(delete_.Content->variable_name);
        delete_.Content->variable->addReference();
    }
    
    void operator()(ast::StructDeclaration& declaration){
        //Delay if necessary
        if(!is_resolved(declaration.Content->variableType)){
            proxy->delayed = true;
            return;
        }

        visit_each(*proxy, declaration.Content->values);

        if(check_variable(declaration.Content->context, declaration.Content->variableName, declaration.Content->position)){
            auto type = visit(ast::TypeTransformer(context), declaration.Content->variableType);

            if(!type->is_custom_type() && !type->is_template()){
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
    
    void operator()(ast::VariableDeclaration& declaration){
        //Delay if necessary
        if(!is_resolved(declaration.Content->variableType)){
            proxy->delayed = true;
            return;
        }
        
        visit_optional(*proxy, declaration.Content->value);

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
            //If it's a template or custom type
            } else {
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
        swap.Content->lhs_var->addReference();
        swap.Content->rhs_var->addReference();
    }

    void operator()(ast::MemberValue& variable){
        visit(*proxy, variable.Content->location);

        auto type = visit(ast::GetTypeVisitor(), variable.Content->location);
        auto struct_name = type->is_pointer() ? type->data_type()->mangle() : type->mangle();
        auto struct_type = context->get_struct(struct_name);

        //Reference the structure
        struct_type->add_reference();

        auto& members = variable.Content->memberNames;
        for(std::size_t i = 0; i < members.size(); ++i){
            auto& member = members[i];

            if(!struct_type->member_exists(member)){ 
                throw SemanticalException("The struct " + struct_name + " has no member named " + member, variable.Content->position);
            }

            //Add a reference to the member
            (*struct_type)[member]->add_reference();

            //If it is not the last member
            if(i != members.size() - 1){
                //The next member will be a member of the current member type
                struct_type = context->get_struct((*struct_type)[member]->type->mangle());
                struct_name = struct_type->name;
            }
        }
    }

    void operator()(ast::VariableValue& variable){
        if (!variable.Content->context->exists(variable.Content->variableName)) {
            throw SemanticalException("Variable " + variable.Content->variableName + " has not been declared", variable.Content->position);
        }

        //Reference the variable
        variable.Content->var = variable.Content->context->getVariable(variable.Content->variableName);
        variable.Content->var->addReference();
    }

    void operator()(ast::ArrayValue& array){
        if (!array.Content->context->exists(array.Content->arrayName)) {
            throw SemanticalException("Array " + array.Content->arrayName + " has not been declared", array.Content->position);
        }
        
        //Reference the variable
        array.Content->var = array.Content->context->getVariable(array.Content->arrayName);
        array.Content->var->addReference();

        visit(*proxy, array.Content->indexValue);
    }

    void operator()(ast::DereferenceValue& variable){
        visit(*proxy, variable.Content->ref);
    }

    void operator()(ast::Expression& value){
        visit(*proxy, value.Content->first);
        
        for_each(value.Content->operations.begin(), value.Content->operations.end(), 
            [&](ast::Operation& operation){ visit(*proxy, operation.get<1>()); });
    }
};

} //end of anonymous namespace

void ast::defineVariables(ast::SourceFile& program){
    VisitorProxy<VariablesVisitor> proxy(program.Content->context);
    proxy.visitor.proxy = &proxy;

    visit_non_variant(proxy, program);
}
