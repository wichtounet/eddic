//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
        std::shared_ptr<ast::TemplateEngine> template_engine;
        std::string mangled_name;

        FunctionCheckerVisitor(std::shared_ptr<ast::TemplateEngine> template_engine, const std::string& mangled_name) : template_engine(template_engine), mangled_name(mangled_name) {}

        void operator()(ast::DefaultCase& default_case){
            check_each(default_case.instructions);
        }

        void operator()(ast::Foreach& foreach){
            template_engine->check_type(foreach.variableType, foreach.position);

            if(check_variable(foreach.context, foreach.variableName, foreach.position)){
                auto type = visit(ast::TypeTransformer(context), foreach.variableType);

                auto var = foreach.context->addVariable(foreach.variableName, type);
                var->set_source_position(foreach.position);
            }

            check_each(foreach.instructions);
        }

        void operator()(ast::ForeachIn& foreach){
            template_engine->check_type(foreach.variableType, foreach.position);

            if(check_variable(foreach.context, foreach.variableName, foreach.position)){
                if(!foreach.context->exists(foreach.arrayName)){
                    throw SemanticalException("The foreach array " + foreach.arrayName  + " has not been declared", foreach.position);
                }

                auto type = visit(ast::TypeTransformer(context), foreach.variableType);

                foreach.var = foreach.context->addVariable(foreach.variableName, type);
                foreach.var->set_source_position(foreach.position);

                foreach.arrayVar = foreach.context->getVariable(foreach.arrayName);
                foreach.iterVar = foreach.context->generate_variable("foreach_iter", INT);

                //Add references to variables
                foreach.var->add_reference();
                foreach.iterVar->add_reference();
                foreach.arrayVar->add_reference();
            }

            check_each(foreach.instructions);
        }

        void operator()(ast::If& if_){
            check_value(if_.condition);
            check_each(if_.instructions);
            visit_each_non_variant(*this, if_.elseIfs);
            visit_optional_non_variant(*this, if_.else_);
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

            variable_value.context = context;
            variable_value.position = position;
            variable_value.variableName = "this";
            variable_value.var = context->getVariable("this");

            return variable_value;
        }

        template<typename V>
        void check_value(V& value){
            if(auto* ptr = boost::smart_relaxed_get<ast::FunctionCall>(&value)){
                auto& functionCall = *ptr;

                template_engine->check_function(functionCall);

                check_each(functionCall.values);

                std::string name = functionCall.function_name;

                auto types = get_types(functionCall);

                auto mangled = mangle(name, types);
                auto original_mangled = mangled;

                for(auto& type : types){
                    if(type->is_structure()){
                        std::vector<std::shared_ptr<const Type>> ctor_types = {new_pointer_type(type)};
                        auto ctor_name = mangle_ctor(ctor_types, type);

                        if(!context->exists(ctor_name)){
                            throw SemanticalException("Passing a structure by value needs a copy constructor", functionCall.position);
                        }
                    }
                }

                if(context->exists(mangled)){
                    functionCall.mangled_name = mangled;
                } else {
                    auto local_context = functionCall.context->function();

                    if(local_context && local_context->struct_type && context->struct_exists(local_context->struct_type->mangle())){
                        auto struct_type = local_context->struct_type;

                        do {
                            mangled = mangle(name, types, struct_type);

                            if(context->exists(mangled)){
                                ast::Cast cast_value;
                                cast_value.resolved_type = new_pointer_type(struct_type);
                                cast_value.value = this_variable(functionCall.context, functionCall.position);

                                ast::FunctionCall function_call_operation;
                                function_call_operation.function_name = functionCall.function_name;
                                function_call_operation.template_types = functionCall.template_types;
                                function_call_operation.values = functionCall.values;
                                function_call_operation.mangled_name = mangled;

                                ast::Expression member_function_call;
                                member_function_call.context = functionCall.context;
                                member_function_call.position = functionCall.position;
                                member_function_call.first = cast_value;
                                member_function_call.operations.push_back(boost::make_tuple(ast::Operator::CALL, function_call_operation));

                                value = member_function_call;

                                (*this)(value);

                                return;
                            }

                            struct_type = context->get_struct(struct_type)->parent_type;
                        } while(struct_type);
                    }

                    throw SemanticalException("The function \"" + unmangle(original_mangled) + "\" does not exists", functionCall.position);
                }
            } else if(auto* ptr = boost::smart_relaxed_get<ast::VariableValue>(&value)){
                auto& variable = *ptr;
                if (!variable.context->exists(variable.variableName)) {
                    auto context = variable.context->function();
                    auto global_context = variable.context->global();

                    if(context && context->struct_type && global_context->struct_exists(context->struct_type->mangle())){
                        auto struct_type = global_context->get_struct(context->struct_type);

                        do {
                            if(struct_type->member_exists(variable.variableName)){
                                ast::Expression member_value;
                                member_value.context = variable.context;
                                member_value.position = variable.position;
                                member_value.first = this_variable(variable.context, variable.position);
                                member_value.operations.push_back(boost::make_tuple(ast::Operator::DOT, variable.variableName)); //TODO CHECK SECOND PARAMETER (SHOULD NOT BE STRING)

                                value = member_value;

                                check_value(value);

                                return;
                            }

                            struct_type = global_context->get_struct(struct_type->parent_type);
                        } while(struct_type);
                    }

                    throw SemanticalException("Variable " + variable.variableName + " has not been declared", variable.position);
                }

                //Reference the variable
                variable.var = variable.context->getVariable(variable.variableName);
                variable.var->add_reference();
            } else {
                visit(*this, value);
            }
        }

        void operator()(ast::For& for_){
            if(for_.start){
                check_value(*for_.start);
            }

            if(for_.condition){
                check_value(*for_.condition);
            }

            if(for_.repeat){
                check_value(*for_.repeat);
            }

            check_each(for_.instructions);
        }

        void operator()(ast::While& while_){
            check_value(while_.condition);
            check_each(while_.instructions);
        }

        void operator()(ast::DoWhile& while_){
            check_value(while_.condition);
            check_each(while_.instructions);
        }

        void operator()(ast::SourceFile& program){
            context = program.context;

            visit_each(*this, program.blocks);
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
            return get_types(functionCall.values);
        }

        void operator()(ast::Switch& switch_){
            check_value(switch_.value);
            visit_each_non_variant(*this, switch_.cases);
            visit_optional_non_variant(*this, switch_.default_case);
        }

        void operator()(ast::SwitchCase& switch_case){
            check_value(switch_case.value);
            check_each(switch_case.instructions);
        }

        void operator()(ast::Assignment& assignment){
            check_value(assignment.left_value);
            check_value(assignment.value);
        }

        void operator()(ast::VariableDeclaration& declaration){
            template_engine->check_type(declaration.variableType, declaration.position);

            if(declaration.value){
                check_value(*declaration.value);
            }

            if(check_variable(declaration.context, declaration.variableName, declaration.position)){
                auto type = visit(ast::TypeTransformer(context), declaration.variableType);

                //If it's a standard type
                if(type->is_standard_type()){
                    if(type->is_const()){
                        if(!declaration.value){
                            throw SemanticalException("A constant variable must have a value", declaration.position);
                        }

                        if(!visit(ast::IsConstantVisitor(), *declaration.value)){
                            throw SemanticalException("The value must be constant", declaration.position);
                        }

                        auto var = declaration.context->addVariable(declaration.variableName, type, *declaration.value);
                        var->set_source_position(declaration.position);
                    } else {
                        auto var = declaration.context->addVariable(declaration.variableName, type);
                        var->set_source_position(declaration.position);
                    }
                }
                //If it's a pointer type
                else if(type->is_pointer()){
                    if(type->is_const()){
                        throw SemanticalException("Pointer types cannot be const", declaration.position);
                    }

                    auto var = declaration.context->addVariable(declaration.variableName, type);
                    var->set_source_position(declaration.position);
                }
                //If it's a array
                else if(type->is_array()){
                    auto var = declaration.context->addVariable(declaration.variableName, type);
                    var->set_source_position(declaration.position);
                }
                //If it's a template or custom type
                else {
                    auto mangled = type->mangle();

                    if(context->struct_exists(mangled)){
                        if(type->is_const()){
                            throw SemanticalException("Custom types cannot be const", declaration.position);
                        }

                        auto var = declaration.context->addVariable(declaration.variableName, type);
                        var->set_source_position(declaration.position);
                    } else {
                        throw SemanticalException("The type \"" + mangled + "\" does not exists", declaration.position);
                    }
                }
            }

            if(declaration.value){
                check_value(*declaration.value);
            }
        }

        void operator()(ast::StructDeclaration& declaration){
            template_engine->check_type(declaration.variableType, declaration.position);

            check_each(declaration.values);

            if(check_variable(declaration.context, declaration.variableName, declaration.position)){
                auto type = visit(ast::TypeTransformer(context), declaration.variableType);

                if(!type->is_custom_type() && !type->is_template_type()){
                    throw SemanticalException("Only custom types take parameters when declared", declaration.position);
                }

                auto mangled = type->mangle();

                if(context->struct_exists(mangled)){
                    if(type->is_const()){
                        throw SemanticalException("Custom types cannot be const", declaration.position);
                    }

                    auto var = declaration.context->addVariable(declaration.variableName, type);
                    var->set_source_position(declaration.position);
                } else {
                    throw SemanticalException("The type \"" + mangled + "\" does not exists", declaration.position);
                }
            }
        }

        template<typename ArrayDeclaration>
        void declare_array(ArrayDeclaration& declaration){
            template_engine->check_type(declaration.arrayType, declaration.position);

            check_value(declaration.size);

            if(check_variable(declaration.context, declaration.arrayName, declaration.position)){
                auto element_type = visit(ast::TypeTransformer(context), declaration.arrayType);

                if(element_type->is_array()){
                    throw SemanticalException("Arrays of arrays are not supported", declaration.position);
                }

                auto constant = visit(ast::IsConstantVisitor(), declaration.size);

                if(!constant){
                    throw SemanticalException("Array size must be constant", declaration.position);
                }

                auto value = visit(ast::GetConstantValue(), declaration.size);
                auto size = boost::smart_get<int>(value);

                auto var = declaration.context->addVariable(declaration.arrayName, new_array_type(element_type, size));
                var->set_source_position(declaration.position);
            }
        }

        void operator()(ast::GlobalArrayDeclaration& declaration){
            declare_array(declaration);
        }

        void operator()(ast::ArrayDeclaration& declaration){
            declare_array(declaration);
        }

        void operator()(ast::PrefixOperation& operation){
            check_value(operation.left_value);
        }

        void operator()(ast::Return& return_){
            return_.mangled_name = mangled_name;

            check_value(return_.value);
        }

        void operator()(ast::Ternary& ternary){
            check_value(ternary.condition);
            check_value(ternary.true_value);
            check_value(ternary.false_value);
        }

        void operator()(ast::BuiltinOperator& builtin){
            check_each(builtin.values);
        }

        void operator()(ast::Cast& cast){
            check_value(cast.value);
        }

        void operator()(ast::Expression& value){
            check_value(value.first);

            auto context = value.context->global();

            auto type = visit(ast::GetTypeVisitor(), value.first);
            for(auto& op : value.operations){
                if(ast::has_operation_value(op)){
                    decltype(auto) op_value = op.get<1>();

                    if(op.get<0>() == ast::Operator::CALL){
                        decltype(auto) function_call = boost::smart_get<ast::FunctionCall>(op_value);
                        check_each(function_call.values);
                    } else {
                        check_value(op.get<1>());
                    }
                }

                template_engine->check_member_function(type, op, value.position);

                if(op.get<0>() == ast::Operator::DOT){
                    auto struct_type = value.context->global()->get_struct(type);
                    auto orig = struct_type;

                    //We delay it
                    if(!struct_type){
                        return;
                    }

                    //Reference the structure
                    struct_type->add_reference();

                    auto& member = boost::smart_get<ast::Literal>(op.get<1>()).value;
                    bool found = false;

                    do {
                        if(struct_type->member_exists(member)){
                            found = true;
                            break;
                        }

                        struct_type = value.context->global()->get_struct(struct_type->parent_type);
                    } while(struct_type);

                    if(!found){
                        throw SemanticalException("The struct " + orig->name + " has no member named " + member, value.position);
                    }

                    //Add a reference to the member
                    (*struct_type)[member].add_reference();
                }

                if(op.get<0>() == ast::Operator::CALL){
                    auto struct_type = type->is_pointer() ? type->data_type() : type;

                    if(!struct_type->is_structure()){
                        throw SemanticalException("Member functions can only be used with structures", value.position);
                    }

                    auto& call_value = boost::smart_get<ast::FunctionCall>(op.get<1>());
                    std::string name = call_value.function_name;

                    auto types = get_types(call_value.values);

                    bool found = false;
                    bool parent = false;
                    std::string mangled;

                    do {
                        mangled = mangle(name, types, struct_type);

                        if(context->exists(mangled)){
                            call_value.mangled_name = mangled;

                            if(parent){
                                call_value.left_type = struct_type;
                            }

                            found = true;
                            break;
                        }

                        struct_type = context->get_struct(struct_type)->parent_type;
                        parent = true;
                    } while(struct_type);

                    if(!found){
                        throw SemanticalException("The member function \"" + unmangle(mangled) + "\" does not exists", value.position);
                    }
                }

                type = ast::operation_type(type, value.context, op);
            }
        }

        void operator()(ast::New& new_){
            check_each(new_.values);
        }

        void operator()(ast::NewArray& new_){
            check_value(new_.size);
        }

        void operator()(ast::Delete& delete_){
            check_value(delete_.value);
        }

        bool check_variable(std::shared_ptr<Context> context, const std::string& name, const ast::Position& position){
            if(context->exists(name)){
                auto var = context->getVariable(name);

                //TODO Comparing the position is not safe enough since
                //parameters and variables generated by the compiler itself
                //always have the same position 0:0:0
                if(var->source_position() == position){
                    return false;
                } else {
                    throw SemanticalException("The Variable " + name + " has already been declared", position);
                }
            }

            return true;
        }

        void operator()(ast::GlobalVariableDeclaration& declaration){
            template_engine->check_type(declaration.variableType, declaration.position);

            if(check_variable(declaration.context, declaration.variableName, declaration.position)){
                if(!visit(ast::IsConstantVisitor(), *declaration.value)){
                    throw SemanticalException("The value must be constant", declaration.position);
                }

                auto type = visit(ast::TypeTransformer(context), declaration.variableType);

                auto var = declaration.context->addVariable(declaration.variableName, type, *declaration.value);
                var->set_source_position(declaration.position);
            }
        }

        bool is_valid(const ast::Type& type){
            if(auto* ptr = boost::smart_get<ast::ArrayType>(&type)){
                return is_valid(ptr->type);
            } else if(auto* ptr = boost::smart_get<ast::SimpleType>(&type)){
                if(is_standard_type(ptr->type)){
                    return true;
                }

                auto t = visit_non_variant(ast::TypeTransformer(context), *ptr);
                return context->struct_exists(t->mangle());
            } else if(auto* ptr = boost::smart_get<ast::PointerType>(&type)){
                return is_valid(ptr->type);
            } else if(auto* ptr = boost::smart_get<ast::TemplateType>(&type)){
                auto t = visit_non_variant(ast::TypeTransformer(context), *ptr);
                return context->struct_exists(t->mangle());
            }

            cpp_unreachable("Invalid type");
        }

        template<typename Function>
        void visit_function(Function& declaration){
            //Add all the parameters to the function context
            for(auto& parameter : declaration.parameters){
                template_engine->check_type(parameter.parameterType, declaration.position);

                if(check_variable(declaration.context, parameter.parameterName, declaration.position)){
                    if(!is_valid(parameter.parameterType)){
                        throw SemanticalException("Invalid parameter type " + ast::to_string(parameter.parameterType), declaration.position);
                    }

                    auto type = visit(ast::TypeTransformer(context), parameter.parameterType);
                    auto var = declaration.context->addParameter(parameter.parameterName, type);
                    var->set_source_position(declaration.position);
                }
            }

            check_each(declaration.instructions);
        }

        void operator()(ast::FunctionCall&){
            cpp_unreachable("Should be handled by check_value");
        }

        void operator()(ast::VariableValue&){
            cpp_unreachable("Should be handled by check_value");
        }

        AUTO_FORWARD()
        AUTO_RECURSE_SCOPE()
        AUTO_IGNORE_OTHERS()
};

} //end of anonymous namespace

void ast::FunctionCheckPass::apply_struct(ast::struct_definition& struct_, bool indicator){
    if(!indicator && context->is_recursively_nested(context->get_struct(struct_.struct_type))){
        context->error_handler.semantical_exception("The structure " + struct_.struct_type->mangle() + " is invalidly nested", struct_);
    }
}

void ast::FunctionCheckPass::apply_function(ast::TemplateFunctionDeclaration& declaration){
    if(!declaration.is_template()){
        FunctionCheckerVisitor visitor(template_engine, declaration.mangledName);
        visitor.context = context;
        visitor.visit_function(declaration);

        auto return_type = visit(ast::TypeTransformer(context), declaration.returnType);
        if(return_type->is_custom_type() || return_type->is_template_type()){
            declaration.context->addParameter("__ret", new_pointer_type(return_type));
        }
    }
}

void ast::FunctionCheckPass::apply_struct_function(ast::TemplateFunctionDeclaration& declaration){
    if(!declaration.is_template()){
        apply_function(declaration);
    }
}

void ast::FunctionCheckPass::apply_struct_constructor(ast::Constructor& constructor){
    FunctionCheckerVisitor visitor(template_engine, constructor.mangledName);
    visitor.context = context;
    visitor.visit_function(constructor);
}

void ast::FunctionCheckPass::apply_struct_destructor(ast::Destructor& destructor){
    FunctionCheckerVisitor visitor(template_engine, destructor.mangledName);
    visitor.context = context;
    visitor.visit_function(destructor);
}

void ast::FunctionCheckPass::apply_program(ast::SourceFile& program, bool indicator){
    context = program.context;

    if(!indicator){
        FunctionCheckerVisitor visitor(template_engine, "");
        visitor.context = context;

        for(auto& block : program.blocks){
            if(auto* ptr = boost::smart_get<ast::GlobalArrayDeclaration>(&block)){
                visit_non_variant(visitor, *ptr);
            } else if(auto* ptr = boost::smart_get<ast::GlobalVariableDeclaration>(&block)){
                visit_non_variant(visitor, *ptr);
            }
        }
    }
}
