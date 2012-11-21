//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <utility>

#include <boost/range/adaptors.hpp>

#include "assert.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"
#include "SemanticalException.hpp"
#include "FunctionContext.hpp"
#include "mangling.hpp"
#include "Labels.hpp"
#include "Type.hpp"
#include "PerfsTimer.hpp"
#include "GlobalContext.hpp"

#include "mtac/Compiler.hpp"
#include "mtac/Program.hpp"
#include "mtac/Printer.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Statement.hpp"

#include "ast/SourceFile.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

namespace {

typedef std::vector<mtac::Argument> arguments;

/* Assignments (left_value = value) */

void assign(mtac::function_p function, ast::Value& left_value, ast::Value& value);
void assign(mtac::function_p function, std::shared_ptr<Variable> left_value, ast::Value& value);

std::shared_ptr<Variable> performBoolOperation(ast::Expression& value, mtac::function_p function);
void performStringOperation(ast::Expression& value, mtac::function_p function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2);
void execute_call(ast::FunctionCall& functionCall, mtac::function_p function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);
mtac::Argument moveToArgument(ast::Value& value, mtac::function_p function);
arguments compile_ternary(mtac::function_p function, ast::Ternary& ternary);
arguments perform_prefix_operation(mtac::function_p function, ast::PrefixOperation& operation);
void pass_arguments(mtac::function_p function, std::shared_ptr<eddic::Function> definition, std::vector<ast::Value>& values);

std::shared_ptr<Variable> performOperation(ast::Expression& value, mtac::function_p function, std::shared_ptr<Variable> t1, mtac::Operator f(ast::Operator)){
    eddic_assert(value.Content->operations.size() > 0, "Operations with no operation should have been transformed before");

    mtac::Argument left = moveToArgument(value.Content->first, function);
    mtac::Argument right;

    //Apply all the operations in chain
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        right = moveToArgument(boost::get<ast::Value>(*operation.get<1>()), function);
       
        if (i == 0){
            function->add(std::make_shared<mtac::Quadruple>(t1, left, f(operation.get<0>()), right));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(t1, t1, f(operation.get<0>()), right));
        }
    }

    return t1;
}

std::shared_ptr<Variable> performIntOperation(ast::Expression& value, mtac::function_p function){
    return performOperation(value, function, function->context->new_temporary(INT), &mtac::toOperator);
}

std::shared_ptr<Variable> performFloatOperation(ast::Expression& value, mtac::function_p function){
    return performOperation(value, function, function->context->new_temporary(FLOAT), &mtac::toFloatOperator);
}

mtac::Argument computeIndexOfArray(std::shared_ptr<Variable> array, ast::Value indexValue, mtac::function_p function){
    mtac::Argument index = moveToArgument(indexValue, function);
    
    auto temp = function->context->new_temporary(INT);

    function->add(std::make_shared<mtac::Quadruple>(temp, index, mtac::Operator::MUL, array->type()->data_type()->size(function->context->global()->target_platform())));
    function->add(std::make_shared<mtac::Quadruple>(temp, temp, mtac::Operator::ADD, INT->size(function->context->global()->target_platform())));
   
    return temp;
}
    
arguments get_member(mtac::function_p function, unsigned int offset, std::shared_ptr<const Type> member_type, std::shared_ptr<Variable> var){
    auto platform = function->context->global()->target_platform();

    if(member_type == STRING){
        auto t1 = function->context->new_temporary(INT);
        auto t2 = function->context->new_temporary(INT);

        function->add(std::make_shared<mtac::Quadruple>(t1, var, mtac::Operator::DOT, offset));
        function->add(std::make_shared<mtac::Quadruple>(t2, var, mtac::Operator::DOT, offset + INT->size(platform)));

        return {t1, t2};
    } else if(member_type->is_array()){
        auto elements = member_type->elements();
        auto data_type = member_type->data_type();

        arguments result;

        for(unsigned int i = 0; i < elements; ++i){
            if(data_type == STRING){
                //TODO
            } else if(data_type->is_custom_type()){
                //TODO
            } else {
                auto temp = function->context->new_temporary(data_type);

                if(data_type == FLOAT){
                    function->add(std::make_shared<mtac::Quadruple>(temp, var, mtac::Operator::FDOT, offset + i * data_type->size(platform)));
                } else if(data_type == INT || data_type == CHAR || data_type == BOOL || data_type->is_pointer()){
                    function->add(std::make_shared<mtac::Quadruple>(temp, var, mtac::Operator::DOT, offset + i * data_type->size(platform)));
                } else {
                    eddic_unreachable("Unhandled type");
                }

                result.push_back(temp);
            }
        }

        return result;
    } else {
        auto temp = function->context->new_temporary(member_type);

        if(member_type == FLOAT){
            function->add(std::make_shared<mtac::Quadruple>(temp, var, mtac::Operator::FDOT, offset));
        } else if(member_type == INT || member_type == CHAR || member_type == BOOL || member_type->is_pointer()){
            function->add(std::make_shared<mtac::Quadruple>(temp, var, mtac::Operator::DOT, offset));
        } else {
            eddic_unreachable("Unhandled type");
        }

        return {temp};
    }
}

template<bool Address>
arguments compute_expression_operation(mtac::function_p function, std::shared_ptr<const Type> type, arguments& left, ast::Operation& operation){
    auto operation_value = operation.get<1>();

    switch(operation.get<0>()){
        case ast::Operator::BRACKET:
            {
                auto index_value = boost::get<ast::Value>(*operation_value);

                if(type == STRING){
                    assert(left.size()  == 1 || left.size() == 2);

                    auto index = moveToArgument(index_value, function);
                    auto pointer_temp = function->context->new_temporary(INT);
                    auto t1 = function->context->new_temporary(INT);

                    //Get the label
                    function->add(std::make_shared<mtac::Quadruple>(pointer_temp, left[0], mtac::Operator::ASSIGN));

                    //Get the specified char 
                    auto quadruple = std::make_shared<mtac::Quadruple>(t1, pointer_temp, mtac::Operator::DOT, index);
                    quadruple->size = mtac::Size::BYTE;
                    function->add(quadruple);

                    left = {t1};
                } else {
                    assert(left.size() == 1);

                    auto index = computeIndexOfArray(boost::get<std::shared_ptr<Variable>>(left[0]), index_value, function); 
                    auto data_type = type->data_type();

                    if(data_type == BOOL || data_type == CHAR || data_type == INT || data_type == FLOAT || data_type->is_pointer()){
                        auto temp = function->context->new_temporary(data_type);
                        function->add(std::make_shared<mtac::Quadruple>(temp, left[0], mtac::Operator::DOT, index));
                        left = {temp};
                    } else if (data_type == STRING){
                        auto t1 = function->context->new_temporary(INT);
                        function->add(std::make_shared<mtac::Quadruple>(t1, left[0], mtac::Operator::DOT, index));

                        auto t2 = function->context->new_temporary(INT);
                        auto t3 = function->context->new_temporary(INT);

                        //Assign the second part of the string
                        function->add(std::make_shared<mtac::Quadruple>(t3, index, mtac::Operator::ADD, INT->size(function->context->global()->target_platform())));
                        function->add(std::make_shared<mtac::Quadruple>(t2, left[0], mtac::Operator::DOT, t3));

                        left = {t1, t2};
                    } else {
                        eddic_unreachable("Type not handled by BRACKET");
                    }
                }
            }

        case ast::Operator::DOT:
            {
                assert(left.size() == 1);
                auto variable = boost::get<std::shared_ptr<Variable>>(left[0]);
                auto member = boost::get<std::string>(*operation_value);

                std::shared_ptr<const Type> member_type;
                unsigned int offset = 0;
                boost::tie(offset, member_type) = mtac::compute_member(function->context->global(), variable, {member});

                if(Address){
                    auto temp = function->context->new_temporary(member_type->is_pointer() ? member_type : new_pointer_type(member_type));

                    function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::PDOT, offset));

                    left = {temp};
                } else {
                    left = get_member(function, offset, member_type, variable);
                }
            }

        case ast::Operator::CALL:
            {
                auto call_operation_value = boost::get<ast::CallOperationValue>(*operation_value);
                auto definition = *call_operation_value.get<4>();

                eddic_assert(definition, "All the member functions should be in the function table");

                auto type = definition->returnType;

                auto left_value = left[0];

                std::shared_ptr<Variable> return_;
                std::shared_ptr<Variable> return2_;

                if(type == BOOL || type == CHAR || type == INT || type == FLOAT || type->is_pointer()){
                    return_ = function->context->new_temporary(type);

                    left = {return_};
                } else if(type == STRING){
                    return_ = function->context->new_temporary(INT);
                    return2_ = function->context->new_temporary(INT);

                    left = {return_, return2_};
                } else {
                    eddic_unreachable("Unhandled function return type");
                }

                //Pass all normal arguments
                pass_arguments(function, definition, call_operation_value.get<2>());

                //Pass the address of the object to the member function
                auto mtac_param = std::make_shared<mtac::Param>(left_value, definition->context->getVariable(definition->parameters[0].name), definition);
                mtac_param->address = true;
                function->add(mtac_param);   

                //Call the function
                function->add(std::make_shared<mtac::Call>(definition->mangledName, definition, return_, return2_));
            }

        default:
            eddic_unreachable("Invalid operator");
    }

    //The computed values by this operation
    return left;
}

template<bool Address = false>
struct ToArgumentsVisitor : public boost::static_visitor<arguments> {
    ToArgumentsVisitor(mtac::function_p f) : function(f) {}
    
    mutable mtac::function_p function;

    result_type operator()(ast::Literal& literal) const {
        return {literal.label, (int) literal.value.size() - 2};
    }

    result_type operator()(ast::CharLiteral& literal) const {
        char v = literal.value[1];

        return {static_cast<int>(v)};
    }

    result_type operator()(ast::Integer& integer) const {
        return {integer.value};
    }
    
    result_type operator()(ast::IntegerSuffix& integer) const {
        return {(double) integer.value};
    }
    
    result_type operator()(ast::Float& float_) const {
        return {float_.value};
    }
    
    result_type operator()(ast::False&) const {
        return {0};
    }
    
    result_type operator()(ast::True&) const {
        return {1};
    }
    
    result_type operator()(ast::Null&) const {
        return {0};
    }

    result_type operator()(ast::New& new_) const {
        auto type = visit(ast::TypeTransformer(function->context->global()), new_.Content->type);
    
        auto param = std::make_shared<mtac::Param>(type->size(function->context->global()->target_platform()));
        param->std_param = "a";
        param->function = function->context->global()->getFunction("_F5allocI");
        function->add(param);

        auto t1 = function->context->new_temporary(new_pointer_type(INT));

        function->context->global()->addReference("_F5allocI");
        function->add(std::make_shared<mtac::Call>("_F5allocI", function->context->global()->getFunction("_F5allocI"), t1)); 
            
        if(type->is_custom_type() || type->is_template()){
            auto ctor_name = mangle_ctor(new_.Content->values, type);

            if(!function->context->global()->exists(ctor_name)){
                assert(new_.Content->values.empty());
            } else {
                auto ctor_function = function->context->global()->getFunction(ctor_name);

                //Pass all normal arguments
                pass_arguments(function, ctor_function, new_.Content->values);

                auto ctor_param = std::make_shared<mtac::Param>(t1, ctor_function->context->getVariable(ctor_function->parameters[0].name), ctor_function);
                ctor_param->address = true;
                function->add(ctor_param);

                function->context->global()->addReference(ctor_name);
                function->add(std::make_shared<mtac::Call>(ctor_name, ctor_function)); 
            }
        }

        return {t1};
    }
    
    result_type operator()(ast::NewArray& new_) const {
        auto type = visit_non_variant(ast::GetTypeVisitor(), new_);

        auto size = function->context->new_temporary(INT);
        auto size_temp = visit(ToArgumentsVisitor<>(function), new_.Content->size)[0];
        
        auto platform = function->context->global()->target_platform();

        function->add(std::make_shared<mtac::Quadruple>(size, size_temp, mtac::Operator::MUL, static_cast<int>(type->data_type()->size(platform))));
        function->add(std::make_shared<mtac::Quadruple>(size, size, mtac::Operator::ADD, static_cast<int>(INT->size(platform))));
    
        auto param = std::make_shared<mtac::Param>(size);
        param->std_param = "a";
        param->function = function->context->global()->getFunction("_F5allocI");
        function->add(param);

        auto t1 = function->context->new_temporary(new_pointer_type(INT));

        function->context->global()->addReference("_F5allocI");
        function->add(std::make_shared<mtac::Call>("_F5allocI", function->context->global()->getFunction("_F5allocI"), t1)); 
        
        function->add(std::make_shared<mtac::Quadruple>(t1, 0, mtac::Operator::DOT_ASSIGN, size_temp));

        return {t1};
    }

    result_type operator()(ast::BuiltinOperator& builtin) const {
        auto& value = builtin.Content->values[0];

        switch(builtin.Content->type){
            case ast::BuiltinType::SIZE:{
                eddic_assert(boost::get<ast::VariableValue>(&value), "The size builtin can only be applied to variable");
                
                auto variable = boost::get<ast::VariableValue>(value).Content->var;

                if(variable->position().isGlobal()){
                    return {variable->type()->elements()};
                } else if((variable->position().is_variable() || variable->position().isStack()) && variable->type()->has_elements()){
                    return {variable->type()->elements()};
                } else if(variable->position().isParameter() || variable->position().isStack() || variable->position().is_variable()){
                    auto t1 = function->context->new_temporary(INT);

                    //The size of the array is at the address pointed by the variable
                    function->add(std::make_shared<mtac::Quadruple>(t1, variable, mtac::Operator::DOT, 0));

                    return {t1};
                }

                eddic_unreachable("The variable is not of a valid type");
            }
            case ast::BuiltinType::LENGTH:
                return {visit(*this, value)[1]};
        }

        eddic_unreachable("This builtin operator is not handled");
    }

    result_type operator()(ast::FunctionCall& call) const {
        auto type = call.Content->function->returnType;

        if(type == BOOL || type == CHAR || type == INT || type == FLOAT || type->is_pointer()){
            auto t1 = function->context->new_temporary(type);

            execute_call(call, function, t1, {});

            return {t1};
        } else if(type == STRING){
            auto t1 = function->context->new_temporary(INT);
            auto t2 = function->context->new_temporary(INT);

            execute_call(call, function, t1, t2);

            return {t1, t2};
        }
        
        eddic_unreachable("Unhandled function return type");
    }

    result_type operator()(ast::Assignment& assignment) const {
        eddic_assert(assignment.Content->op == ast::Operator::ASSIGN, "Compound assignment should be transformed into Assignment");

        assign(function, assignment.Content->left_value, assignment.Content->value);

        return visit(*this, assignment.Content->left_value);
    }

    result_type operator()(ast::Ternary& ternary) const {
        return compile_ternary(function, ternary);
    }

    result_type operator()(std::shared_ptr<Variable> var) const {
        return {var};
    }

    result_type operator()(ast::VariableValue& value) const {
        if(Address){
            return {value.Content->var};
        }

        auto type = value.Content->var->type();

        //If it's a const, we just have to replace it by its constant value
        if(type->is_const()){
            auto val = value.Content->var->val();
            auto nc_type = type;//->non_const();

            if(nc_type == INT || nc_type == BOOL){
                return {boost::get<int>(val)};
            } else if(nc_type == CHAR){
                return {boost::get<char>(val)};        
            } else if(nc_type == FLOAT){
                return {boost::get<double>(val)};        
            } else if(nc_type == STRING){
                auto value = boost::get<std::pair<std::string, int>>(val);

                return {value.first, value.second};
            } 

            eddic_unreachable("void is not a type");
        } else if(type->is_array() || type->is_pointer()){
            return {value.Content->var};
        } else {
            if(type == INT || type == CHAR || type == BOOL || type == FLOAT){
                return {value.Content->var};
            } else if(type == STRING){
                auto temp = value.Content->context->new_temporary(INT);
                function->add(std::make_shared<mtac::Quadruple>(temp, value.Content->var, mtac::Operator::DOT, INT->size(function->context->global()->target_platform())));

                return {value.Content->var, temp};
            } else if(type->is_custom_type() || type->is_template()) {
                //If we are here, it means that we want to pass it by reference
                return {value.Content->var};
            } 
        }
    
        eddic_unreachable("Unhandled type");
    }

    result_type operator()(ast::PrefixOperation& value) const {
        return perform_prefix_operation(function, value);
    }

    result_type operator()(ast::Expression& value) const {
        auto type = visit(ast::GetTypeVisitor(), value.Content->first);

        //TODO Perhaps this special handling should be integrated in the compute_expression function above
        auto& first_operation = value.Content->operations[0];
        auto first_op = first_operation.get<0>();

        if(first_op == ast::Operator::ADD || first_op == ast::Operator::SUB || first_op == ast::Operator::DIV || first_op == ast::Operator::MUL || 
                first_op == ast::Operator::MOD || first_op == ast::Operator::AND || first_op == ast::Operator::OR){
            if(type == INT){
                return {performIntOperation(value, function)};
            } else if(type == FLOAT){
                return {performFloatOperation(value, function)};
            } else if(type == BOOL){
                return {performBoolOperation(value, function)};
            } else if(type == STRING){
                auto t1 = function->context->new_temporary(INT);
                auto t2 = function->context->new_temporary(INT);

                performStringOperation(value, function, t1, t2);

                return {t1, t2};
            }
        }
        
        auto left = visit(*this, value.Content->first);

        //Compute each operation
        for(auto& operation : value.Content->operations){
            //Execute the current operation
            left = compute_expression_operation<Address>(function, type, left, operation);

            //Get the type computed by the current operation for the next one
            type = ast::operation_type(type, value.Content->context, operation);
        }

        //Return the value returned by the last operation
        return left;
    }

    result_type operator()(ast::Cast& cast) const {
        mtac::Argument arg = moveToArgument(cast.Content->value, function);
        
        auto srcType = visit(ast::GetTypeVisitor(), cast.Content->value);
        auto destType = visit(ast::TypeTransformer(cast.Content->context->global()), cast.Content->type);

        if(srcType != destType){
            auto t1 = function->context->new_temporary(destType);

            if(destType == FLOAT){
                function->add(std::make_shared<mtac::Quadruple>(t1, arg, mtac::Operator::I2F));
            } else if(destType == INT){
                if(srcType == FLOAT){
                    function->add(std::make_shared<mtac::Quadruple>(t1, arg, mtac::Operator::F2I));
                } else if(srcType == CHAR){
                    function->add(std::make_shared<mtac::Quadruple>(t1, arg, mtac::Operator::ASSIGN));
                }
            } else if(destType == CHAR){
                function->add(std::make_shared<mtac::Quadruple>(t1, arg, mtac::Operator::ASSIGN));
            } else {
                return {arg};
            }
            
            return {t1};
        }

        //If srcType == destType, there is nothing to do
        return {arg};
    }
};

struct AbstractVisitor : public boost::static_visitor<> {
    AbstractVisitor(mtac::function_p f) : function(f) {}
    
    mutable mtac::function_p function;
    
    virtual void intAssign(const arguments& arguments) const = 0;
    virtual void pointerAssign(const arguments& arguments) const = 0;
    virtual void floatAssign(const arguments& arguments) const = 0;
    virtual void stringAssign(const arguments& arguments) const = 0;
    
    template<typename T>
    void complexAssign(std::shared_ptr<const Type> type, T& value) const {
        if(type->is_pointer()){
            pointerAssign(ToArgumentsVisitor<>(function)(value));
        } else if(type->is_array() || type == INT || type == CHAR || type == BOOL){
            intAssign(ToArgumentsVisitor<>(function)(value));
        } else if(type == STRING){
            stringAssign(ToArgumentsVisitor<>(function)(value));
        } else if(type == FLOAT){
            floatAssign(ToArgumentsVisitor<>(function)(value));
        } else {
            eddic_unreachable("Unhandled variable type");
        }
    }

    template<typename T>
    void operator()(T& value) const {
        complexAssign(ast::GetTypeVisitor()(value), value);
    }
};

struct AssignValueToVariable : public AbstractVisitor {
    AssignValueToVariable(mtac::function_p f, std::shared_ptr<Variable> v) : AbstractVisitor(f), variable(v) {}
    AssignValueToVariable(mtac::function_p f, std::shared_ptr<Variable> v, ast::Value& indexValue) : AbstractVisitor(f), variable(v), indexValue(indexValue) {}
    AssignValueToVariable(mtac::function_p f, std::shared_ptr<Variable> v, unsigned int offset) : AbstractVisitor(f), variable(v), offset(offset) {}
    AssignValueToVariable(mtac::function_p f, std::shared_ptr<Variable> v, unsigned int offset, std::shared_ptr<const Type> type) : AbstractVisitor(f), variable(v), type(type), offset(offset) {}
    
    std::shared_ptr<Variable> variable;
    std::shared_ptr<const Type> type;
    boost::optional<unsigned int> offset;
    boost::optional<ast::Value> indexValue;

    void intAssign(const arguments& arguments) const {
        if(offset){
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset, mtac::Operator::DOT_ASSIGN, arguments[0]));
        } else if(indexValue){
            auto index = computeIndexOfArray(variable, *indexValue, function); 
            function->add(std::make_shared<mtac::Quadruple>(variable, index, mtac::Operator::DOT_ASSIGN, arguments[0]));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(variable, arguments[0], mtac::Operator::ASSIGN));
        }
    }

    void pointerAssign(const arguments& arguments) const {
        if(offset){
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset, mtac::Operator::DOT_PASSIGN, arguments[0]));
        } else if(indexValue){
            auto index = computeIndexOfArray(variable, *indexValue, function); 
            function->add(std::make_shared<mtac::Quadruple>(variable, index, mtac::Operator::DOT_PASSIGN, arguments[0]));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(variable, arguments[0], mtac::Operator::PASSIGN));
        }
    }

    void floatAssign(const arguments& arguments) const {
        if(offset){
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset, mtac::Operator::DOT_FASSIGN, arguments[0]));
        } else if(indexValue){
            auto index = computeIndexOfArray(variable, *indexValue, function); 
            function->add(std::make_shared<mtac::Quadruple>(variable, index, mtac::Operator::DOT_FASSIGN, arguments[0]));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(variable, arguments[0], mtac::Operator::FASSIGN));
        }
    }

    void stringAssign(const arguments& arguments) const {
        if(offset){
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset, mtac::Operator::DOT_ASSIGN, arguments[0]));
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset + INT->size(function->context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, arguments[1]));
        } else if(indexValue){
            auto index = computeIndexOfArray(variable, *indexValue, function); 

            function->add(std::make_shared<mtac::Quadruple>(variable, index, mtac::Operator::DOT_ASSIGN, arguments[0]));

            auto temp1 = function->context->new_temporary(INT);
            function->add(std::make_shared<mtac::Quadruple>(temp1, index, mtac::Operator::ADD, INT->size(function->context->global()->target_platform())));
            function->add(std::make_shared<mtac::Quadruple>(variable, temp1, mtac::Operator::DOT_ASSIGN, arguments[1]));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(variable, arguments[0], mtac::Operator::ASSIGN));
            function->add(std::make_shared<mtac::Quadruple>(variable, INT->size(function->context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, arguments[1]));
        }
    }
    
    /*void operator()(ast::MemberValue& value) const {
        if(type){
            if(type->is_pointer()){
                pointerAssign(ToArgumentsVisitor<true>(function)(value));
            } else {
                complexAssign(type, value);
            }
        } else {
            complexAssign(visit_non_variant(ast::GetTypeVisitor(), value), value);
        }
    }*/
    
    void operator()(ast::VariableValue& value) const {
        if(type){
            if(type->is_pointer()){
                pointerAssign(ToArgumentsVisitor<true>(function)(value));
            } else {
                complexAssign(type, value);
            }
        } else {
            if(variable->type()->is_array() && variable->type()->data_type()->is_pointer()){
                pointerAssign(ToArgumentsVisitor<>(function)(value));
            } else {
                complexAssign(value.variable()->type(), value);
            }
        }
    }

    template<typename T>
    void operator()(T& value) const {
        complexAssign(ast::GetTypeVisitor()(value), value);
    }
};

struct DereferenceAssign : public AbstractVisitor {
    DereferenceAssign(mtac::function_p f, std::shared_ptr<Variable> v, unsigned int offset) : AbstractVisitor(f), variable(v), offset(offset) {}
    
    std::shared_ptr<Variable> variable;
    unsigned int offset;

    void intAssign(const arguments& arguments) const {
        if(offset == 0){
            function->add(std::make_shared<mtac::Quadruple>(variable, 0, mtac::Operator::DOT_ASSIGN, arguments[0]));
        } else {
            auto temp = function->context->new_temporary(new_pointer_type(INT));

            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::DOT, offset));
            function->add(std::make_shared<mtac::Quadruple>(temp, 0, mtac::Operator::DOT_ASSIGN, arguments[0]));
        }
    }
    
    void pointerAssign(const arguments& arguments) const {
        if(offset == 0){
            function->add(std::make_shared<mtac::Quadruple>(variable, 0, mtac::Operator::DOT_PASSIGN, arguments[0]));
        } else {
            auto temp = function->context->new_temporary(new_pointer_type(INT));

            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::DOT, offset));
            function->add(std::make_shared<mtac::Quadruple>(temp, 0, mtac::Operator::DOT_PASSIGN, arguments[0]));
        }
    }
    
    void floatAssign(const arguments& arguments) const {
        if(offset == 0){
            function->add(std::make_shared<mtac::Quadruple>(variable, 0, mtac::Operator::DOT_FASSIGN, arguments[0]));
        } else {
            auto temp = function->context->new_temporary(new_pointer_type(INT));

            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::FDOT, offset));
            function->add(std::make_shared<mtac::Quadruple>(temp, 0, mtac::Operator::DOT_FASSIGN, arguments[0]));
        }
    }

    void stringAssign(const arguments& arguments) const {
        if(offset == 0){
            function->add(std::make_shared<mtac::Quadruple>(variable, 0, mtac::Operator::DOT_ASSIGN, arguments[0]));
            function->add(std::make_shared<mtac::Quadruple>(variable, INT->size(function->context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, arguments[1]));
        } else {
            auto temp = function->context->new_temporary(new_pointer_type(INT));
            
            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::DOT, offset));

            function->add(std::make_shared<mtac::Quadruple>(temp, offset, mtac::Operator::DOT_ASSIGN, arguments[0]));
            function->add(std::make_shared<mtac::Quadruple>(temp, offset + INT->size(function->context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, arguments[1]));
        }
    }
};

struct AssignVisitor : public boost::static_visitor<> {
    mtac::function_p function;
    ast::Value& value;

    AssignVisitor(mtac::function_p function, ast::Value& value) : function(function), value(value) {}

    /*void operator()(ast::MemberValue& member_value){
        std::shared_ptr<Variable> source, dest;

        if(auto* ptr = boost::get<ast::VariableValue>(&member_value.Content->location)){
            source = dest = (*ptr).Content->var;
        } else if(auto* ptr = boost::get<ast::ArrayValue>(&member_value.Content->location)){
            auto left = *ptr;

            if(auto* ref_ptr = boost::get<ast::VariableValue>(&left.Content->ref)){
                auto& ref_var = *ref_ptr;

                source = ref_var.Content->var;

                auto index = computeIndexOfArray(source, left.Content->indexValue, function); 

                dest = ref_var.Content->context->new_temporary(new_pointer_type(INT));
                function->add(std::make_shared<mtac::Quadruple>(dest, source, mtac::Operator::PDOT, index));
            } else {
                //TODO
            }
        } else if(auto* ptr = boost::get<ast::MemberValue>(&member_value.Content->location)){
            auto visitor = ToArgumentsVisitor<true>(function);
            auto left_value = visit_non_variant(visitor, *ptr);
            auto variable = boost::get<std::shared_ptr<Variable>>(left_value[0]);

            source = dest = variable;
        } else {
            eddic_unreachable("Unhandled location type");
        }

        unsigned int offset = 0;
        std::shared_ptr<const Type> member_type;
        boost::tie(offset, member_type) = mtac::compute_member(function->context->global(), source, member_value.Content->memberNames);

        visit(AssignValueToVariable(function, dest, offset, member_type), value);
    }*/

    void operator()(ast::VariableValue& variable_value){
        auto variable = variable_value.Content->var;

        visit(AssignValueToVariable(function, variable), value);
    }

    void operator()(ast::Expression& expression){
        /*TODO if(auto* ref_ptr = boost::get<ast::VariableValue>(&array_value.Content->ref)){
            visit(AssignValueToVariable(function, ref_ptr->Content->var, array_value.Content->indexValue), value);
        } else {
            //TODO
        }*/
    }

    void operator()(ast::PrefixOperation& dereference_value){
        if(dereference_value.Content->op == ast::Operator::STAR){
            /*if(auto* var_ptr = boost::get<ast::MemberValue>(&dereference_value.Content->left_value)){
                auto member_value = *var_ptr;

                if(auto* ptr = boost::get<ast::VariableValue>(&member_value.Content->location)){
                    auto left = *ptr;

                    auto variable = left.Content->var;
                    unsigned int offset = mtac::compute_member_offset(function->context->global(), variable, member_value.Content->memberNames);

                    visit(DereferenceAssign(function, variable, offset), value);
                }
                } else if(boost::get<ast::ArrayValue>(&member_value.Content->location)){
                    eddic_unreachable("Unhandled location");
                }
            } else if(auto* var_ptr = boost::get<ast::VariableValue>(&dereference_value.Content->left_value)){
                auto left = *var_ptr;

                visit(DereferenceAssign(function, left.Content->var, 0), value);
            } else if(auto* array_ptr = boost::get<ast::ArrayValue>(&dereference_value.Content->left_value)){
                auto left = *array_ptr;

                //As the array hold pointers, the visitor will return a temporary
                auto visitor = ToArgumentsVisitor<>(function);
                auto values = visit_non_variant(visitor, left);

                eddic_assert(mtac::isVariable(values[0]), "The visitor should return a temporary variable");

                auto variable = boost::get<std::shared_ptr<Variable>>(values[0]);

                visit(DereferenceAssign(function, variable, 0), value);
            } else {
                eddic_unreachable("Unsupported type"); 
            }*/
        } else {
            eddic_unreachable("Unsupported left value");
        }
    }
    
    template<typename T>
    result_type operator()(T&){
        eddic_unreachable("Unsupported left value");
    }
};

//TODO Remove the old visitors once the Assignment visitor is finished

struct AssignmentVisitor : public boost::static_visitor<> {
    mtac::function_p function;
    ast::Value& right_value;

    AssignmentVisitor(mtac::function_p function, ast::Value& right_value) : function(function), right_value(right_value) {}

    void operator()(ast::VariableValue& variable_value){
        auto variable = variable_value.Content->var;
        auto type = visit(ast::GetTypeVisitor(), right_value); 
        auto values = visit(ToArgumentsVisitor<>(function), right_value);

        if(type->is_pointer() || (variable->type()->is_array() && variable->type()->data_type()->is_pointer())){
            function->add(std::make_shared<mtac::Quadruple>(variable, values[0], mtac::Operator::PASSIGN));
        } else if(type->is_array() || type == INT || type == CHAR || type == BOOL){
            function->add(std::make_shared<mtac::Quadruple>(variable, values[0], mtac::Operator::ASSIGN));
        } else if(type == STRING){
            function->add(std::make_shared<mtac::Quadruple>(variable, values[0], mtac::Operator::ASSIGN));
            function->add(std::make_shared<mtac::Quadruple>(variable, INT->size(function->context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, values[1]));
        } else if(type == FLOAT){
            function->add(std::make_shared<mtac::Quadruple>(variable, values[0], mtac::Operator::FASSIGN));
        } else {
            eddic_unreachable("Unhandled value type");
        }
    }
    
    /*template<typename T>
    void complexAssign(std::shared_ptr<const Type> type, T& value) const {
        if(type->is_pointer()){
            pointerAssign(ToArgumentsVisitor<>(function)(value));
        } else if(type->is_array() || type == INT || type == CHAR || type == BOOL){
            intAssign(ToArgumentsVisitor<>(function)(value));
        } else if(type == STRING){
            stringAssign(ToArgumentsVisitor<>(function)(value));
        } else if(type == FLOAT){
            floatAssign(ToArgumentsVisitor<>(function)(value));
        } else {
            eddic_unreachable("Unhandled variable type");
        }
    }*/

    void operator()(ast::Expression& value){
        auto left = visit(ToArgumentsVisitor<>(function), value.Content->first);
        auto type = visit(ast::GetTypeVisitor(), value.Content->first);

        //Compute each operation but the last
        for(std::size_t i = 0; i < value.Content->operations.size() - 1; ++i){
            auto& operation = value.Content->operations[i];

            //Execute the current operation
            left = compute_expression_operation<false>(function, type, left, operation);

            //Get the type computed by the current operation for the next one
            type = ast::operation_type(type, value.Content->context, operation);
        }

        //Assign the right value to the left value generated by 

        auto& last_operation = value.Content->operations.back();

        //Assign to an element of an array
        if(last_operation.get<0>() == ast::Operator::BRACKET){
            assert(mtac::isVariable(left[0]));
            auto array_variable = boost::get<std::shared_ptr<Variable>>(left[0]);

            auto& index_value = boost::get<ast::Value>(*last_operation.get<1>());
            auto index = computeIndexOfArray(array_variable, index_value, function); 
        
            auto values = visit(ToArgumentsVisitor<>(function), right_value);

            if(type->is_pointer()){
                function->add(std::make_shared<mtac::Quadruple>(array_variable, index, mtac::Operator::DOT_PASSIGN, values[0]));
            } else if(type->is_array() || type == INT || type == CHAR || type == BOOL){
                function->add(std::make_shared<mtac::Quadruple>(array_variable, index, mtac::Operator::DOT_ASSIGN, values[0]));
            } else if(type == STRING){
                function->add(std::make_shared<mtac::Quadruple>(array_variable, index, mtac::Operator::DOT_ASSIGN, values[0]));

                auto temp1 = function->context->new_temporary(INT);
                function->add(std::make_shared<mtac::Quadruple>(temp1, index, mtac::Operator::ADD, INT->size(function->context->global()->target_platform())));
                function->add(std::make_shared<mtac::Quadruple>(array_variable, temp1, mtac::Operator::DOT_ASSIGN, values[1]));
            } else if(type == FLOAT){
                function->add(std::make_shared<mtac::Quadruple>(array_variable, index, mtac::Operator::DOT_FASSIGN, values[0]));
            } else {
                eddic_unreachable("Unhandled value type");
            }
        } 
        //Assign to a member of a struct
        else if(last_operation.get<0>() == ast::Operator::DOT){
            //TODO
        } else {
            eddic_unreachable("This postfix operator does not result in a left value");
        }
    }

    void operator()(ast::PrefixOperation& dereference_value){
        if(dereference_value.Content->op == ast::Operator::STAR){
            //TODO
        } else {
            eddic_unreachable("This prefix operator does not result in a left value");
        }
    }
    
    template<typename T>
    void operator()(T&){
        eddic_unreachable("Not a left value");
    }
};

void assign(mtac::function_p function, ast::Value& left_value, ast::Value& value){
    AssignVisitor visitor(function, value);
    visit(visitor, left_value);
}

void assign(mtac::function_p function, std::shared_ptr<Variable> variable, ast::Value& value){
    ast::VariableValue left_value;
    left_value.Content->var = variable;
    left_value.Content->variableName = variable->name();
    left_value.Content->context = function->context;

    AssignVisitor visitor(function, value);
    visit_non_variant(visitor, left_value);
}

struct JumpIfFalseVisitor : public boost::static_visitor<> {
    JumpIfFalseVisitor(mtac::function_p f, const std::string& l) : function(f), label(l) {}
    
    mutable mtac::function_p function;
    std::string label;
   
    void operator()(ast::Expression& value) const ;
    
    template<typename T>
    void operator()(T& value) const {
        auto argument = ToArgumentsVisitor<>(function)(value)[0];

        function->add(std::make_shared<mtac::IfFalse>(argument, label));
    }
};

template<typename Control>
void compare(ast::Expression& value, ast::Operator op, mtac::function_p function, const std::string& label){
    eddic_assert(value.Content->operations.size() == 1, "Relational operations cannot be chained");

    auto left = moveToArgument(value.Content->first, function);
    auto right = moveToArgument(boost::get<ast::Value>(*value.Content->operations[0].get<1>()), function);

    auto typeLeft = visit(ast::GetTypeVisitor(), value.Content->first);

    if(typeLeft == INT || typeLeft == CHAR || typeLeft->is_pointer()){
        function->add(std::make_shared<Control>(mtac::toBinaryOperator(op), left, right, label));
    } else if(typeLeft == FLOAT){
        function->add(std::make_shared<Control>(mtac::toFloatBinaryOperator(op), left, right, label));
    } 
}

struct JumpIfTrueVisitor : public boost::static_visitor<> {
    JumpIfTrueVisitor(mtac::function_p f, const std::string& l) : function(f), label(l) {}
    
    mutable mtac::function_p function;
    std::string label;
   
    void operator()(ast::Expression& value) const {
        auto op = value.Content->operations[0].get<0>();

        //Logical and operators (&&)
        if(op == ast::Operator::AND){
            std::string codeLabel = newLabel();

            visit(JumpIfFalseVisitor(function, codeLabel), value.Content->first);

            for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
                if(i == value.Content->operations.size() - 1){
                    visit(*this, boost::get<ast::Value>(*value.Content->operations[i].get<1>()));   
                } else {
                    visit(JumpIfFalseVisitor(function, codeLabel), boost::get<ast::Value>(*value.Content->operations[i].get<1>()));
                }
            }

            function->add(codeLabel);
        } 
        //Logical or operators (||)
        else if(op == ast::Operator::OR){
            visit(*this, value.Content->first);

            for(auto& operation : value.Content->operations){
                visit(*this, boost::get<ast::Value>(*operation.get<1>()));
            }
        }
        //Relational operators 
        else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
            compare<mtac::If>(value, op, function, label);
        } 
        //A bool value
        else { //Perform int operations
            auto var = performIntOperation(value, function);
            
            function->add(std::make_shared<mtac::If>(var, label));
        }
    }
   
    template<typename T>
    void operator()(T& value) const {
        auto argument = ToArgumentsVisitor<>(function)(value)[0];

        function->add(std::make_shared<mtac::If>(argument, label));
    }
};

void JumpIfFalseVisitor::operator()(ast::Expression& value) const {
    auto op = value.Content->operations[0].get<0>();

    //Logical and operators (&&)
    if(op == ast::Operator::AND){
        visit(*this, value.Content->first);

        for(auto& operation : value.Content->operations){
            visit(*this, boost::get<ast::Value>(*operation.get<1>()));
        }
    } 
    //Logical or operators (||)
    else if(op == ast::Operator::OR){
        std::string codeLabel = newLabel();

        visit(JumpIfTrueVisitor(function, codeLabel), value.Content->first);

        for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
            if(i == value.Content->operations.size() - 1){
                visit(*this, boost::get<ast::Value>(*value.Content->operations[i].get<1>()));
            } else {
                visit(JumpIfTrueVisitor(function, codeLabel), boost::get<ast::Value>(*value.Content->operations[i].get<1>()));
            }
        }

        function->add(codeLabel);
    }
    //Relational operators 
    else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
        compare<mtac::IfFalse>(value, op, function, label);
    } 
    //A bool value
    else { //Perform int operations
        auto var = performIntOperation(value, function);

        function->add(std::make_shared<mtac::IfFalse>(var, label));
    }
}

arguments compile_ternary(mtac::function_p function, ast::Ternary& ternary){
    auto type = visit_non_variant(ast::GetTypeVisitor(), ternary);

    auto falseLabel = newLabel();
    auto endLabel = newLabel();

    if(type == INT || type == CHAR || type == BOOL || type == FLOAT){
        auto t1 = function->context->new_temporary(type);

        visit(JumpIfFalseVisitor(function, falseLabel), ternary.Content->condition); 
        assign(function, t1, ternary.Content->true_value);
        function->add(std::make_shared<mtac::Goto>(endLabel));
        
        function->add(falseLabel);
        assign(function, t1, ternary.Content->false_value);
        
        function->add(endLabel);

        return {t1};
    } else if(type == STRING){
        auto t1 = function->context->new_temporary(INT);
        auto t2 = function->context->new_temporary(INT);
        
        visit(JumpIfFalseVisitor(function, falseLabel), ternary.Content->condition); 
        auto args = visit(ToArgumentsVisitor<>(function), ternary.Content->true_value);
        function->add(std::make_shared<mtac::Quadruple>(t1, args[0], mtac::Operator::ASSIGN));  
        function->add(std::make_shared<mtac::Quadruple>(t2, args[1], mtac::Operator::ASSIGN));  

        function->add(std::make_shared<mtac::Goto>(endLabel));
        
        function->add(falseLabel);
        args = visit(ToArgumentsVisitor<>(function), ternary.Content->false_value);
        function->add(std::make_shared<mtac::Quadruple>(t1, args[0], mtac::Operator::ASSIGN));  
        function->add(std::make_shared<mtac::Quadruple>(t2, args[1], mtac::Operator::ASSIGN));  
        
        function->add(endLabel);
        
        return {t1, t2};
    }

    eddic_unreachable("Unhandled ternary type");
}

void performStringOperation(ast::Expression& value, mtac::function_p function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2){
    eddic_assert(value.Content->operations.size() > 0, "Expression with no operation should have been transformed");

    arguments arguments;

    auto first = visit(ToArgumentsVisitor<>(function), value.Content->first);
    arguments.insert(arguments.end(), first.begin(), first.end());

    //Perfom all the additions
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        auto second = visit(ToArgumentsVisitor<>(function), boost::get<ast::Value>(*operation.get<1>()));
        arguments.insert(arguments.end(), second.begin(), second.end());
        
        for(auto& arg : arguments){
            function->add(std::make_shared<mtac::Param>(arg));   
        }

        arguments.clear();
        
        function->context->global()->addReference("_F6concatSS");

        if(i == value.Content->operations.size() - 1){
            function->add(std::make_shared<mtac::Call>("_F6concatSS", function->context->global()->getFunction("_F6concatSS"), v1, v2)); 
        } else {
            auto t1 = function->context->new_temporary(INT);
            auto t2 = function->context->new_temporary(INT);
            
            function->add(std::make_shared<mtac::Call>("_F6concatSS", function->context->global()->getFunction("_F6concatSS"), t1, t2)); 
          
            arguments.push_back(t1);
            arguments.push_back(t2);
        }
    }
}

class CompilerVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<StringPool> pool;
        mtac::program_p program;
        mtac::function_p function;
    
    public:
        CompilerVisitor(std::shared_ptr<StringPool> p, mtac::program_p mtacProgram) : pool(p), program(mtacProgram) {}

        AUTO_RECURSE_STRUCT()

        //No code is generated for these nodes
        AUTO_IGNORE_TEMPLATE_FUNCTION_DECLARATION()
        AUTO_IGNORE_TEMPLATE_STRUCT()
        AUTO_IGNORE_GLOBAL_VARIABLE_DECLARATION()
        AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
        AUTO_IGNORE_ARRAY_DECLARATION()
        AUTO_IGNORE_IMPORT()
        AUTO_IGNORE_STANDARD_IMPORT()
       
        void operator()(ast::Assignment& assignment){
            eddic_assert(assignment.Content->op == ast::Operator::ASSIGN, "Compound assignment should be transformed into Assignment");

            assign(function, assignment.Content->left_value, assignment.Content->value);
        }
        
        void operator()(ast::SourceFile& p){
            program->context = p.Content->context;

            visit_each(*this, p.Content->blocks);
        }

        void issue_destructors(std::shared_ptr<Context> context){
            for(auto& pair : *context){
                auto var = pair.second;

                if(var->position().isStack() || var->position().is_variable()){
                    auto type = var->type();

                    if(type->is_custom_type() || type->is_template()){
                        auto dtor_name = mangle_dtor(type);

                        //If there is a destructor, call it
                        if(program->context->exists(dtor_name)){
                            auto dtor_function = program->context->getFunction(dtor_name);

                            auto dtor_param = std::make_shared<mtac::Param>(var, dtor_function->context->getVariable(dtor_function->parameters[0].name), dtor_function);
                            dtor_param->address = true;
                            function->add(dtor_param);

                            program->context->addReference(dtor_name);
                            function->add(std::make_shared<mtac::Call>(dtor_name, dtor_function)); 
                        }
                    }
                }
            }
        }

        template<typename Function>
        inline void issue_function(Function& f){
            function = std::make_shared<mtac::Function>(f.Content->context, f.Content->mangledName);
            function->definition = program->context->getFunction(f.Content->mangledName);

            visit_each(*this, f.Content->instructions);

            issue_destructors(f.Content->context);

            program->functions.push_back(function);
        }

        void operator()(ast::FunctionDeclaration& f){
            if(f.Content->functionName == "main" || program->context->referenceCount(f.Content->mangledName) > 0){
                issue_function(f);
            }
        }

        void operator()(ast::Constructor& f){
            issue_function(f);
        }

        void operator()(ast::Destructor& f){
            issue_function(f);
        }

        void operator()(ast::If& if_){
            if (if_.Content->elseIfs.empty()) {
                std::string endLabel = newLabel();

                visit(JumpIfFalseVisitor(function, endLabel), if_.Content->condition);

                visit_each(*this, if_.Content->instructions);

                issue_destructors(if_.Content->context);

                if (if_.Content->else_) {
                    std::string elseLabel = newLabel();

                    function->add(std::make_shared<mtac::Goto>(elseLabel));

                    function->add(endLabel);

                    visit_each(*this, (*if_.Content->else_).instructions);
                    
                    issue_destructors((*if_.Content->else_).context);

                    function->add(elseLabel);
                } else {
                    function->add(endLabel);
                }
            } else {
                std::string end = newLabel();
                std::string next = newLabel();

                visit(JumpIfFalseVisitor(function, next), if_.Content->condition);

                visit_each(*this, if_.Content->instructions);
                
                issue_destructors(if_.Content->context);

                function->add(std::make_shared<mtac::Goto>(end));

                for (std::vector<ast::ElseIf>::size_type i = 0; i < if_.Content->elseIfs.size(); ++i) {
                    ast::ElseIf& elseIf = if_.Content->elseIfs[i];

                    function->add(next);

                    //Last elseif
                    if (i == if_.Content->elseIfs.size() - 1) {
                        if (if_.Content->else_) {
                            next = newLabel();
                        } else {
                            next = end;
                        }
                    } else {
                        next = newLabel();
                    }

                    visit(JumpIfFalseVisitor(function, next), elseIf.condition);

                    visit_each(*this, elseIf.instructions);
                    
                    issue_destructors(elseIf.context);

                    function->add(std::make_shared<mtac::Goto>(end));
                }

                if (if_.Content->else_) {
                    function->add(next);

                    visit_each(*this, (*if_.Content->else_).instructions);
                    
                    issue_destructors((*if_.Content->else_).context);
                }

                function->add(end);
            }
        }
        
        void operator()(ast::StructDeclaration& declaration){
            auto var = declaration.Content->context->getVariable(declaration.Content->variableName);
            auto ctor_name = mangle_ctor(declaration.Content->values, var->type());

            if(program->context->exists(ctor_name)){
                auto ctor_function = program->context->getFunction(ctor_name);
                
                //Pass all normal arguments
                pass_arguments(function, ctor_function, declaration.Content->values);

                auto ctor_param = std::make_shared<mtac::Param>(var, ctor_function->context->getVariable(ctor_function->parameters[0].name), ctor_function);
                ctor_param->address = true;
                function->add(ctor_param);

                program->context->addReference(ctor_name);
                function->add(std::make_shared<mtac::Call>(ctor_name, ctor_function)); 
            }
        }

        void operator()(ast::VariableDeclaration& declaration){
            auto var = declaration.Content->context->getVariable(declaration.Content->variableName);

            if(var->type()->is_custom_type() || var->type()->is_template()){
                auto ctor_name = mangle_ctor({}, var->type());

                if(program->context->exists(ctor_name)){
                    auto ctor_function = program->context->getFunction(ctor_name);

                    auto ctor_param = std::make_shared<mtac::Param>(var, ctor_function->context->getVariable(ctor_function->parameters[0].name), ctor_function);
                    ctor_param->address = true;
                    function->add(ctor_param);

                    program->context->addReference(ctor_name);
                    function->add(std::make_shared<mtac::Call>(ctor_name, ctor_function)); 
                }
            } else {
                if(declaration.Content->value){
                    if(!var->type()->is_const()){
                        assign(function, var, *declaration.Content->value);
                    }
                }
            }
        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;
            
            auto t1 = swap.Content->context->new_temporary(INT);

            if(lhs_var->type() == INT || lhs_var->type() == CHAR || lhs_var->type() == BOOL || lhs_var->type() == STRING){
                function->add(std::make_shared<mtac::Quadruple>(t1, rhs_var, mtac::Operator::ASSIGN));  
                function->add(std::make_shared<mtac::Quadruple>(rhs_var, lhs_var, mtac::Operator::ASSIGN));  
                function->add(std::make_shared<mtac::Quadruple>(lhs_var, t1, mtac::Operator::ASSIGN));  
                
                if(lhs_var->type() == STRING){
                    auto t2 = swap.Content->context->new_temporary(INT);

                    //t1 = 4(b)
                    function->add(std::make_shared<mtac::Quadruple>(t1, rhs_var, mtac::Operator::DOT, INT->size(function->context->global()->target_platform())));  
                    //t2 = 4(a)
                    function->add(std::make_shared<mtac::Quadruple>(t2, lhs_var, mtac::Operator::DOT, INT->size(function->context->global()->target_platform())));  
                    //4(b) = t2
                    function->add(std::make_shared<mtac::Quadruple>(rhs_var, INT->size(function->context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, t2));  
                    //4(a) = t1
                    function->add(std::make_shared<mtac::Quadruple>(lhs_var, INT->size(function->context->global()->target_platform()), mtac::Operator::DOT_ASSIGN, t1));  
                }
            } else {
                eddic_unreachable("Unhandled variable type");
            }
        }
        
        void operator()(ast::Expression& expression){
            visit_non_variant(ToArgumentsVisitor<>(function), expression);
        }

        void operator()(ast::PrefixOperation& operation){
            perform_prefix_operation(function, operation);
        }

        void operator()(ast::DoWhile& while_){
            std::string startLabel = newLabel();

            function->add(startLabel);

            visit_each(*this, while_.Content->instructions);

            issue_destructors(while_.Content->context);

            visit(JumpIfTrueVisitor(function, startLabel), while_.Content->condition);
        }

        void operator()(ast::FunctionCall& functionCall){
            execute_call(functionCall, function, {}, {});
        }

        void operator()(ast::Return& return_){
            auto arguments = visit(ToArgumentsVisitor<>(function), return_.Content->value);

            if(arguments.size() == 1){
                function->add(std::make_shared<mtac::Quadruple>(mtac::Operator::RETURN, arguments[0]));
            } else if(arguments.size() == 2){
                function->add(std::make_shared<mtac::Quadruple>(mtac::Operator::RETURN, arguments[0], arguments[1]));
            } else {
                eddic_unreachable("Unhandled arguments size");
            }   
        }

        void operator()(ast::Delete& delete_){
            auto type = delete_.Content->variable->type()->data_type();
            if(type->is_custom_type() || type->is_template()){
                auto dtor_name = mangle_dtor(type);

                //If there is a destructor, call it
                if(program->context->exists(dtor_name)){
                    auto dtor_function = program->context->getFunction(dtor_name);

                    auto dtor_param = std::make_shared<mtac::Param>(delete_.Content->variable, dtor_function->context->getVariable(dtor_function->parameters[0].name), dtor_function);
                    dtor_param->address = true;
                    function->add(dtor_param);

                    program->context->addReference(dtor_name);
                    function->add(std::make_shared<mtac::Call>(dtor_name, dtor_function)); 
                }
            }

            auto free_name = "_F4freePI";
            auto free_function = program->context->getFunction(free_name);

            auto param = std::make_shared<mtac::Param>(delete_.Content->variable);
            param->std_param = "a";
            param->function = free_function;
            function->add(param);

            program->context->addReference(free_name);
            function->add(std::make_shared<mtac::Call>(free_name, free_function)); 
        }

        template<typename T>
        void operator()(T&){
            eddic_unreachable("This element should have been transformed"); 
        }
};

mtac::Argument moveToArgument(ast::Value& value, mtac::function_p function){
    return visit(ToArgumentsVisitor<>(function), value)[0];
}
    
/*void push_struct_member(ast::MemberValue& memberValue, std::shared_ptr<const Type> type, mtac::function_p function, boost::variant<std::shared_ptr<Variable>, std::string> param, std::shared_ptr<Function> definition){
    auto struct_name = type->mangle();
    auto struct_type = function->context->global()->get_struct(struct_name);

    for(auto& member : boost::adaptors::reverse(struct_type->members)){
        auto member_type = member->type;

        memberValue.Content->memberNames.push_back(member->name);

        if(member_type->is_custom_type() || member_type->is_template()){
            push_struct_member(memberValue, member_type, function, param, definition);
        } else {
            auto member_values = ToArgumentsVisitor<>(function)(memberValue);

            for(auto& v : boost::adaptors::reverse(member_values)){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param)){
                    function->add(std::make_shared<mtac::Param>(v, *ptr, definition));
                } else if(auto* ptr = boost::get<std::string>(&param)){
                    function->add(std::make_shared<mtac::Param>(v, *ptr, definition));
                }
            }
        }

        memberValue.Content->memberNames.pop_back();
    }
}*/

void push_struct(mtac::function_p function, boost::variant<std::shared_ptr<Variable>, std::string> param, std::shared_ptr<Function> definition, ast::VariableValue& value){
    auto var = value.Content->var;
    auto context = value.Content->context;

    auto struct_name = var->type()->mangle();
    auto struct_type = function->context->global()->get_struct(struct_name);

    for(auto& member : boost::adaptors::reverse(struct_type->members)){
        auto type = member->type;

        ast::VariableValue variable_value;
        variable_value.Content->context = context;
        variable_value.Content->variableName = var->name();
        variable_value.Content->var = var;
        
        /*ast::MemberValue member_value;
        member_value.Content->context = context;
        member_value.Content->location = variable_value;
        member_value.Content->memberNames = {member->name};

        if(type->is_custom_type() || type->is_template()){
            push_struct_member(member_value, type, function, param, definition);
        } else {
            auto member_values = ToArgumentsVisitor<>(function)(member_value);

            for(auto& v : boost::adaptors::reverse(member_values)){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param)){
                    function->add(std::make_shared<mtac::Param>(v, *ptr, definition));
                } else if(auto* ptr = boost::get<std::string>(&param)){
                    function->add(std::make_shared<mtac::Param>(v, *ptr, definition));
                }
            }
        }*/
    }
}

void pass_arguments(mtac::function_p function, std::shared_ptr<eddic::Function> definition, std::vector<ast::Value>& values){
    auto context = definition->context;

    //If it's a standard function, there are no context
    if(!context){
        auto parameters = definition->parameters;
        int i = parameters.size()-1;

        for(auto& first : boost::adaptors::reverse(values)){
            auto param = parameters[i--].name; 
            
            auto args = visit(ToArgumentsVisitor<>(function), first);
            for(auto& arg : boost::adaptors::reverse(args)){
                function->add(std::make_shared<mtac::Param>(arg, param, definition));   
            }
        }
    } else {
        auto parameters = definition->parameters;
        int i = parameters.size()-1;

        for(auto& first : boost::adaptors::reverse(values)){
            std::shared_ptr<Variable> param = context->getVariable(parameters[i--].name);

            if(auto* ptr = boost::get<ast::VariableValue>(&first)){
                auto type = (*ptr).Content->var->type();
                if((type->is_custom_type() || type->is_template()) && !param->type()->is_pointer()){
                    push_struct(function, param, definition, *ptr);
                    continue;
                }
            } 

            arguments args;
            if(param->type()->is_pointer()){
                args = visit(ToArgumentsVisitor<true>(function), first);
            } else {
                args = visit(ToArgumentsVisitor<>(function), first);
            }
            
            for(auto& arg : boost::adaptors::reverse(args)){
                auto mtac_param = std::make_shared<mtac::Param>(arg, param, definition);
                mtac_param->address = param->type()->is_pointer();

                function->add(mtac_param);
            }
        }
    }
}

void execute_call(ast::FunctionCall& functionCall, mtac::function_p function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_){
    std::shared_ptr<eddic::Function> definition;
    definition = functionCall.Content->function;

    eddic_assert(definition, "All the functions should be in the function table");

    pass_arguments(function, definition, functionCall.Content->values);

    function->add(std::make_shared<mtac::Call>(definition->mangledName, definition, return_, return2_));
}

std::shared_ptr<Variable> performBoolOperation(ast::Expression& value, mtac::function_p function){
    auto t1 = function->context->new_temporary(INT); 
   
    //The first operator defines the kind of operation 
    auto op = value.Content->operations[0].get<0>();

    //Logical and operators (&&)
    if(op == ast::Operator::AND){
        auto falseLabel = newLabel();
        auto endLabel = newLabel();

        visit(JumpIfFalseVisitor(function, falseLabel), value.Content->first);

        for(auto& operation : value.Content->operations){
            visit(JumpIfFalseVisitor(function, falseLabel), boost::get<ast::Value>(*operation.get<1>()));
        }

        function->add(std::make_shared<mtac::Quadruple>(t1, 1, mtac::Operator::ASSIGN));
        function->add(std::make_shared<mtac::Goto>(endLabel));

        function->add(falseLabel);
        function->add(std::make_shared<mtac::Quadruple>(t1, 0, mtac::Operator::ASSIGN));

        function->add(endLabel);
    } 
    //Logical or operators (||)
    else if(op == ast::Operator::OR){
        auto trueLabel = newLabel();
        auto endLabel = newLabel();

        visit(JumpIfTrueVisitor(function, trueLabel), value.Content->first);

        for(auto& operation : value.Content->operations){
            visit(JumpIfTrueVisitor(function, trueLabel), boost::get<ast::Value>(*operation.get<1>()));
        }

        function->add(std::make_shared<mtac::Quadruple>(t1, 0, mtac::Operator::ASSIGN));
        function->add(std::make_shared<mtac::Goto>(endLabel));

        function->add(trueLabel);
        function->add(std::make_shared<mtac::Quadruple>(t1, 1, mtac::Operator::ASSIGN));

        function->add(endLabel);
    }
    //Relational operators 
    else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
        eddic_assert(value.Content->operations.size() == 1, "Relational operations cannot be chained");

        auto left = moveToArgument(value.Content->first, function);
        auto right = moveToArgument(boost::get<ast::Value>(*value.Content->operations[0].get<1>()), function);
        
        auto typeLeft = visit(ast::GetTypeVisitor(), value.Content->first);
        if(typeLeft == INT || typeLeft == CHAR || typeLeft->is_pointer()){
            function->add(std::make_shared<mtac::Quadruple>(t1, left, mtac::toRelationalOperator(op), right));
        } else if(typeLeft == FLOAT){
            function->add(std::make_shared<mtac::Quadruple>(t1, left, mtac::toFloatRelationalOperator(op), right));
        } else {
            eddic_unreachable("Unsupported type in relational operator");
        }
    } else {
        eddic_unreachable("Unsupported operator");
    }
    
    return t1;
}

arguments dereference_variable(mtac::function_p function, std::shared_ptr<Variable> variable, std::shared_ptr<const Type> type){
    if(type == INT || type == CHAR || type == BOOL){
        auto temp = function->context->new_temporary(type);

        function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::DOT, 0));

        return {temp};
    } else if(type == FLOAT){
        auto temp = function->context->new_temporary(type);

        function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::FDOT, 0));

        return {temp};
    } else if(type == STRING){
        auto t1 = function->context->new_temporary(INT);
        auto t2 = function->context->new_temporary(INT);

        function->add(std::make_shared<mtac::Quadruple>(t1, variable, mtac::Operator::DOT, 0));
        function->add(std::make_shared<mtac::Quadruple>(t2, variable, mtac::Operator::DOT, INT->size(function->context->global()->target_platform())));

        return {t1, t2};
    } 

    eddic_unreachable("Unhandled type");
}

arguments perform_prefix_operation(mtac::function_p function, ast::PrefixOperation& operation){
    auto op = operation.Content->op;
    auto type = visit(ast::GetTypeVisitor(), operation.Content->left_value);
    auto left = visit(ToArgumentsVisitor<>(function), operation.Content->left_value);
        
    switch(op){
        case ast::Operator::STAR:
        {
            eddic_assert(left.size() == 1, "STAR only support one value");
            eddic_assert(mtac::isVariable(left[0]), "The visitor should return a temporary variable");

            auto variable = boost::get<std::shared_ptr<Variable>>(left[0]);
            return dereference_variable(function, variable, type->data_type());
        }

        case ast::Operator::NOT:
        {
            eddic_assert(left.size() == 1, "NOT only support one value");

            auto t1 = function->context->new_temporary(BOOL);

            function->add(std::make_shared<mtac::Quadruple>(t1, left[0], mtac::Operator::NOT));

            return {t1};
        }

        case ast::Operator::ADD:
            return left;

        case ast::Operator::SUB:
        {
            eddic_assert(left.size() == 1, "SUB only support one value");

            auto t1 = function->context->new_temporary(type);

            if(type == FLOAT){
                function->add(std::make_shared<mtac::Quadruple>(t1, left[0], mtac::Operator::FMINUS));
            } else {
                function->add(std::make_shared<mtac::Quadruple>(t1, left[0], mtac::Operator::MINUS));
            }

            return {t1};
        }
        
        case ast::Operator::INC:
        {
            eddic_assert(mtac::isVariable(left[0]), "The visitor should return a variable");

            auto t1 = boost::get<std::shared_ptr<Variable>>(left[0]);

            if(type == FLOAT){
                function->add(std::make_shared<mtac::Quadruple>(t1, t1, mtac::Operator::FADD, 1.0));
            } else if (type == INT){
                function->add(std::make_shared<mtac::Quadruple>(t1, t1, mtac::Operator::ADD, 1));
            } 

            return {t1};
        }
        
        case ast::Operator::DEC:
        {
            eddic_assert(mtac::isVariable(left[0]), "The visitor should return a variable");

            auto t1 = boost::get<std::shared_ptr<Variable>>(left[0]);

            if(type == FLOAT){
                function->add(std::make_shared<mtac::Quadruple>(t1, t1, mtac::Operator::FSUB, 1.0));
            } else if (type == INT){
                function->add(std::make_shared<mtac::Quadruple>(t1, t1, mtac::Operator::SUB, 1));
            }

            return {t1};
        }

        default:
            eddic_unreachable("Unsupported operator");    
    }
}

} //end of anonymous namespace

void mtac::Compiler::compile(ast::SourceFile& program, std::shared_ptr<StringPool> pool, mtac::program_p mtacProgram) const {
    PerfsTimer timer("MTAC Compilation");

    CompilerVisitor visitor(pool, mtacProgram);
    visitor(program);
}
