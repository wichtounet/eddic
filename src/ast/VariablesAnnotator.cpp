//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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
#include "ast/TemplateEngine.hpp"

using namespace eddic;

namespace {

struct ValueVisitor : public boost::static_visitor<ast::Value> {
    std::shared_ptr<GlobalContext> context;
    std::shared_ptr<ast::TemplateEngine> template_engine;

    ValueVisitor(std::shared_ptr<GlobalContext> context, std::shared_ptr<ast::TemplateEngine> template_engine) : context(context), template_engine(template_engine) {}
    
    void replace_each(std::vector<ast::Value>& values){
        for(std::size_t i = 0; i < values.size(); ++i){
            values[i] = visit(*this, values[i]);
        }
    }

    ast::Value operator()(ast::FunctionCall& functionCall){
        replace_each(functionCall.Content->values);

        return functionCall;
    }

    ast::Value operator()(ast::Ternary& ternary){
        ternary.Content->condition = visit(*this, ternary.Content->condition);
        ternary.Content->true_value = visit(*this, ternary.Content->true_value);
        ternary.Content->false_value = visit(*this, ternary.Content->false_value);

        return ternary;
    }

    ast::Value operator()(ast::BuiltinOperator& builtin){
        replace_each(builtin.Content->values);

        return builtin;
    }

    ast::Value operator()(ast::Unary& value){
        value.Content->value = visit(*this, value.Content->value);

        return value;
    }

    ast::Value operator()(ast::Cast& cast){
        cast.Content->value = visit(*this, cast.Content->value);

        return cast;
    }

    ast::Value operator()(ast::PrefixOperation& operation){
        operation.Content->left_value = visit(*this, operation.Content->left_value);

        return operation;
    }

    ast::Value operator()(ast::SuffixOperation& operation){
        operation.Content->left_value = visit(*this, operation.Content->left_value);

        return operation;
    }

    //Warning : The location of the MemberValue is not modified
    //TODO If there are new transformation in the future, adapt the following function

    ast::Value operator()(ast::MemberValue& variable){
        auto location = variable.Content->location;

        if(auto* ptr = boost::get<ast::VariableValue>(&location)){
            auto location_variable = *ptr;
            bool transformed = false;

            if (!location_variable.Content->context->exists(location_variable.Content->variableName)) {
                auto context = location_variable.Content->context->function();
                auto global_context = location_variable.Content->context->global();

                if(context && context->struct_type && global_context->struct_exists(context->struct_type->mangle())){
                    auto struct_type = global_context->get_struct(context->struct_type->mangle());

                    if(struct_type->member_exists(location_variable.Content->variableName)){
                        ast::VariableValue this_variable;
                        this_variable.Content->context = location_variable.Content->context;
                        this_variable.Content->var = location_variable.Content->context->getVariable("this");
                        this_variable.Content->variableName = "this";
                        this_variable.Content->position = location_variable.Content->position;

                        variable.Content->location = this_variable;
                        variable.Content->memberNames.insert(variable.Content->memberNames.begin(), location_variable.Content->variableName);

                        transformed = true;
                    }
                }
            }
            
            if(!transformed){
                visit_non_variant(*this, location_variable);
            }
        } else if(auto* ptr = boost::get<ast::ArrayValue>(&location)){
            visit_non_variant(*this, *ptr);
        } else {
            eddic_unreachable("Not a left value");
        }

        auto type = visit(ast::GetTypeVisitor(), variable.Content->location);
        auto struct_name = type->is_pointer() ? type->data_type()->mangle() : type->mangle();
        auto struct_type = context->get_struct(struct_name);

        //We delay it
        if(!struct_type){
            return variable;
        }

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

                //We delay it
                if(!struct_type){
                    return variable;
                }

                struct_name = struct_type->name;
            }
        }

        return variable;
    }

    ast::Value operator()(ast::VariableValue& variable){
        if (!variable.Content->context->exists(variable.Content->variableName)) {
            auto context = variable.Content->context->function();
            auto global_context = variable.Content->context->global();

            if(context && context->struct_type && global_context->struct_exists(context->struct_type->mangle())){
                auto struct_type = global_context->get_struct(context->struct_type->mangle());
                
                if(struct_type->member_exists(variable.Content->variableName)){
                    ast::VariableValue this_variable;
                    this_variable.Content->context = variable.Content->context;
                    this_variable.Content->var = variable.Content->context->getVariable("this");
                    this_variable.Content->variableName = "this";
                    this_variable.Content->position = variable.Content->position;

                    ast::MemberValue member_value;
                    member_value.Content->context = variable.Content->context;
                    member_value.Content->position = variable.Content->position;
                    member_value.Content->location = this_variable;
                    member_value.Content->memberNames.push_back(variable.Content->variableName);

                    return visit_non_variant(*this, member_value);
                }
            }

            throw SemanticalException("Variable " + variable.Content->variableName + " has not been declared", variable.Content->position);
        }

        //Reference the variable
        variable.Content->var = variable.Content->context->getVariable(variable.Content->variableName);
        variable.Content->context->add_reference(variable.Content->var);

        return variable;
    }

    ast::Value operator()(ast::ArrayValue& array){
        array.Content->ref = visit(*this, array.Content->ref);
        array.Content->indexValue = visit(*this, array.Content->indexValue);

        return array;
    }

    ast::Value operator()(ast::DereferenceValue& variable){
        variable.Content->ref = visit(*this, variable.Content->ref);

        return variable;
    }

    ast::Value operator()(ast::Expression& value){
        value.Content->first = visit(*this, value.Content->first);
        
        for_each(value.Content->operations.begin(), value.Content->operations.end(), 
            [&](ast::Operation& operation){ operation.get<1>() = visit(*this, operation.get<1>()); });

        return value;
    }

    ast::Value operator()(ast::MemberFunctionCall& functionCall){
        functionCall.Content->object = visit(*this, functionCall.Content->object);

        replace_each(functionCall.Content->values);

        return functionCall;
    }

    ast::Value operator()(ast::Assignment& assignment){
        assignment.Content->left_value = visit(*this, assignment.Content->left_value);
        assignment.Content->value = visit(*this, assignment.Content->value);

        return assignment;
    }
    
    ast::Value operator()(ast::New& new_){
        replace_each(new_.Content->values);

        return new_;
    }

    AUTO_RETURN_OTHERS(ast::Value)
};

struct VariablesVisitor : public boost::static_visitor<> {
    ValueVisitor value_visitor;

    std::shared_ptr<GlobalContext> context;
    std::shared_ptr<ast::TemplateEngine> template_engine;

    VariablesVisitor(std::shared_ptr<GlobalContext> context, std::shared_ptr<ast::TemplateEngine> template_engine) : 
            value_visitor(context, template_engine), context(context), template_engine(template_engine) {
                //NOP
    }

    AUTO_RECURSE_ELSE()
    AUTO_RECURSE_DEFAULT_CASE()

    void operator()(ast::Return& return_){
        return_.Content->value = visit(value_visitor, return_.Content->value);
    }

    void operator()(ast::For& for_){
        visit_optional(*this, for_.Content->start);

        if(for_.Content->condition){
            for_.Content->condition = visit(value_visitor, *for_.Content->condition);
        }

        visit_optional(*this, for_.Content->repeat);
        visit_each(*this, for_.Content->instructions);
    }
    
    void operator()(ast::While& while_){
        while_.Content->condition = visit(value_visitor, while_.Content->condition);
        visit_each(*this, while_.Content->instructions);
    }
    
    void operator()(ast::DoWhile& while_){
        while_.Content->condition = visit(value_visitor, while_.Content->condition);
        visit_each(*this, while_.Content->instructions);
    }

    void operator()(ast::If& if_){
        if_.Content->condition = visit(value_visitor, if_.Content->condition);
        visit_each(*this, if_.Content->instructions);
        visit_each_non_variant(*this, if_.Content->elseIfs);
        visit_optional_non_variant(*this, if_.Content->else_);
    }

    void operator()(ast::ElseIf& elseIf){
        elseIf.condition = visit(value_visitor, elseIf.condition);
        visit_each(*this, elseIf.instructions);
    }

    void operator()(ast::Switch& switch_){
        switch_.Content->value = visit(value_visitor, switch_.Content->value);
        visit_each_non_variant(*this, switch_.Content->cases);
        visit_optional_non_variant(*this, switch_.Content->default_case);
    }

    void operator()(ast::SwitchCase& switch_case){
        switch_case.value = visit(value_visitor, switch_case.value);
        visit_each(*this, switch_case.instructions);
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

        visit_each(*this, declaration.Content->instructions);
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

    template<typename ArrayDeclaration>
    void declare_array(ArrayDeclaration& declaration){
        template_engine->check_type(declaration.Content->arrayType, declaration.Content->position);

        declaration.Content->size = visit(value_visitor, declaration.Content->size);

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
        template_engine->check_type(foreach.Content->variableType, foreach.Content->position);

        if(check_variable(foreach.Content->context, foreach.Content->variableName, foreach.Content->position)){
            auto type = visit(ast::TypeTransformer(context), foreach.Content->variableType);

            auto var = foreach.Content->context->addVariable(foreach.Content->variableName, type);
            var->set_source_position(foreach.Content->position);
        }

        visit_each(*this, foreach.Content->instructions);
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
            foreach.Content->context->add_reference(foreach.Content->var);
            foreach.Content->context->add_reference(foreach.Content->iterVar);
            foreach.Content->context->add_reference(foreach.Content->arrayVar);
        }

        visit_each(*this, foreach.Content->instructions);
    }

    void operator()(ast::Delete& delete_){
        if (!delete_.Content->context->exists(delete_.Content->variable_name)) {
            throw SemanticalException("Variable " + delete_.Content->variable_name + " has not been declared", delete_.Content->position);
        }
        
        delete_.Content->variable = delete_.Content->context->getVariable(delete_.Content->variable_name);
        delete_.Content->context->add_reference(delete_.Content->variable);
    }
    
    void operator()(ast::StructDeclaration& declaration){
        template_engine->check_type(declaration.Content->variableType, declaration.Content->position);

        value_visitor.replace_each(declaration.Content->values);

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
        template_engine->check_type(declaration.Content->variableType, declaration.Content->position);
        
        if(declaration.Content->value){
            declaration.Content->value = visit(value_visitor, *declaration.Content->value);
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
        swap.Content->context->add_reference(swap.Content->lhs_var);
        swap.Content->context->add_reference(swap.Content->rhs_var);
    }

    /* Forward to value visitor, don't care about return value, as only variable are transformed */
    
    void operator()(ast::FunctionCall& function_call){
        visit_non_variant(value_visitor, function_call);
    }
    
    void operator()(ast::MemberFunctionCall& function_call){
        visit_non_variant(value_visitor, function_call);
    }
    
    void operator()(ast::Assignment& assignment){
        visit_non_variant(value_visitor, assignment);
    }

    void operator()(ast::PrefixOperation& operation){
        visit_non_variant(value_visitor, operation);
    }

    void operator()(ast::SuffixOperation& operation){
        visit_non_variant(value_visitor, operation);
    }
};

} //end of anonymous namespace
    
void ast::VariableAnnotationPass::apply_struct(ast::Struct& struct_, bool indicator){
    if(!indicator && context->is_recursively_nested(struct_.Content->struct_type->mangle())){
        throw SemanticalException("The structure " + struct_.Content->struct_type->mangle() + " is invalidly nested", struct_.Content->position);
    }
}

void ast::VariableAnnotationPass::apply_function(ast::FunctionDeclaration& function){
    VariablesVisitor visitor(context, template_engine);
    visitor.visit_function(function);
}

void ast::VariableAnnotationPass::apply_struct_function(ast::FunctionDeclaration& function){
    VariablesVisitor visitor(context, template_engine);
    visitor.visit_function(function);
}

void ast::VariableAnnotationPass::apply_struct_constructor(ast::Constructor& constructor){
    VariablesVisitor visitor(context, template_engine);
    visitor.visit_function(constructor);
}

void ast::VariableAnnotationPass::apply_struct_destructor(ast::Destructor& destructor){
    VariablesVisitor visitor(context, template_engine);
    visitor.visit_function(destructor);
}

void ast::VariableAnnotationPass::apply_program(ast::SourceFile& program, bool indicator){
    context = program.Content->context;

    if(!indicator){
        VariablesVisitor visitor(context, template_engine);

        for(auto& block : program.Content->blocks){
            if(auto* ptr = boost::get<ast::GlobalArrayDeclaration>(&block)){
                visit_non_variant(visitor, *ptr);
            } else if(auto* ptr = boost::get<ast::GlobalVariableDeclaration>(&block)){
                visit_non_variant(visitor, *ptr);
            }
        }
    }
}
