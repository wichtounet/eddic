//=======================================================================
// Copyright Baptiste Wicht 2011.
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
#include "SymbolTable.hpp"
#include "SemanticalException.hpp"
#include "FunctionContext.hpp"
#include "mangling.hpp"
#include "Labels.hpp"
#include "Type.hpp"
#include "PerfsTimer.hpp"

#include "mtac/Compiler.hpp"
#include "mtac/Program.hpp"
#include "mtac/Printer.hpp"
#include "mtac/Utils.hpp"

#include "ast/SourceFile.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"
#include "ast/ASTVisitor.hpp"

using namespace eddic;

namespace {

//TODO Visitors should be moved out of this class in a future clenaup phase

std::shared_ptr<Variable> performBoolOperation(ast::Expression& value, std::shared_ptr<mtac::Function> function);
void performStringOperation(ast::Expression& value, std::shared_ptr<mtac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2);
void execute_call(ast::FunctionCall& functionCall, std::shared_ptr<mtac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);
void execute_member_call(ast::MemberFunctionCall& functionCall, std::shared_ptr<mtac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);
mtac::Argument moveToArgument(ast::Value& value, std::shared_ptr<mtac::Function> function);
void assign(std::shared_ptr<mtac::Function> function, ast::Assignment& assignment);
std::vector<mtac::Argument> compile_ternary(std::shared_ptr<mtac::Function> function, ast::Ternary& ternary);

std::shared_ptr<Variable> performOperation(ast::Expression& value, std::shared_ptr<mtac::Function> function, std::shared_ptr<Variable> t1, mtac::Operator f(ast::Operator)){
    ASSERT(value.Content->operations.size() > 0, "Operations with no operation should have been transformed before");

    mtac::Argument left = moveToArgument(value.Content->first, function);
    mtac::Argument right;

    //Apply all the operations in chain
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        right = moveToArgument(operation.get<1>(), function);
       
        if (i == 0){
            function->add(std::make_shared<mtac::Quadruple>(t1, left, f(operation.get<0>()), right));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(t1, t1, f(operation.get<0>()), right));
        }
    }

    return t1;
}

std::shared_ptr<Variable> performIntOperation(ast::Expression& value, std::shared_ptr<mtac::Function> function){
    return performOperation(value, function, function->context->newTemporary(), &mtac::toOperator);
}

std::shared_ptr<Variable> performFloatOperation(ast::Expression& value, std::shared_ptr<mtac::Function> function){
    return performOperation(value, function, function->context->newFloatTemporary(), &mtac::toFloatOperator);
}

mtac::Argument computeIndexOfArray(std::shared_ptr<Variable> array, ast::Value indexValue, std::shared_ptr<mtac::Function> function){
    mtac::Argument index = moveToArgument(indexValue, function);
    
    auto temp = function->context->newTemporary();
    auto position = array->position();

    if(position.isGlobal()){
        function->add(std::make_shared<mtac::Quadruple>(temp, index, mtac::Operator::MUL, -1 * array->type()->data_type()->size()));
        function->add(std::make_shared<mtac::Quadruple>(temp, temp, mtac::Operator::ADD, array->type()->data_type()->size() * array->type()->elements()));
        function->add(std::make_shared<mtac::Quadruple>(temp, temp, mtac::Operator::SUB, INT->size()));
    } else if(position.isStack()){
        function->add(std::make_shared<mtac::Quadruple>(temp, index, mtac::Operator::MUL, array->type()->data_type()->size()));
        function->add(std::make_shared<mtac::Quadruple>(temp, temp, mtac::Operator::ADD, INT->size()));
        function->add(std::make_shared<mtac::Quadruple>(temp, temp, mtac::Operator::MUL, -1));
    } else if(position.isParameter()){
        function->add(std::make_shared<mtac::Quadruple>(temp, index, mtac::Operator::MUL, array->type()->data_type()->size()));
        function->add(std::make_shared<mtac::Quadruple>(temp, temp, mtac::Operator::ADD, INT->size()));
        function->add(std::make_shared<mtac::Quadruple>(temp, temp, mtac::Operator::MUL, -1));
    }
   
    return temp;
}

int getStringOffset(std::shared_ptr<Variable> variable){
    return INT->size();
}

template<typename Operation>
void performPrefixOperation(const Operation& operation, std::shared_ptr<mtac::Function> function){
    auto var = operation.Content->variable;

    if(operation.Content->op == ast::Operator::INC){
        if(var->type() == FLOAT){
            function->add(std::make_shared<mtac::Quadruple>(var, var, mtac::Operator::FADD, 1.0));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(var, var, mtac::Operator::ADD, 1));
        }
    } else if(operation.Content->op == ast::Operator::DEC){
        if(var->type() == FLOAT){
            function->add(std::make_shared<mtac::Quadruple>(var, var, mtac::Operator::FSUB, 1.0));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(var, var, mtac::Operator::SUB, 1));
        }
    }
}

template<typename Operation>
std::shared_ptr<Variable> performSuffixOperation(const Operation& operation, std::shared_ptr<mtac::Function> function){
    auto var = operation.Content->variable;

    if(var->type() == FLOAT){
        auto temp = operation.Content->context->newFloatTemporary();

        function->add(std::make_shared<mtac::Quadruple>(temp, var, mtac::Operator::FASSIGN));

        if(operation.Content->op == ast::Operator::INC){
            function->add(std::make_shared<mtac::Quadruple>(var, var, mtac::Operator::FADD, 1.0));
        } else if(operation.Content->op == ast::Operator::DEC){
            function->add(std::make_shared<mtac::Quadruple>(var, var, mtac::Operator::FSUB, 1.0));
        }

        return temp;
    } else if(var->type() == INT){
        auto temp = operation.Content->context->newTemporary();

        function->add(std::make_shared<mtac::Quadruple>(temp, var, mtac::Operator::ASSIGN));

        if(operation.Content->op == ast::Operator::INC){
            function->add(std::make_shared<mtac::Quadruple>(var, var, mtac::Operator::ADD, 1));
        } else if(operation.Content->op == ast::Operator::DEC){
            function->add(std::make_shared<mtac::Quadruple>(var, var, mtac::Operator::SUB, 1));
        }

        return temp;
    } else {
        ASSERT_PATH_NOT_TAKEN("Unhandled type");
    }
}

struct ToArgumentsVisitor : public boost::static_visitor<std::vector<mtac::Argument>> {
    ToArgumentsVisitor(std::shared_ptr<mtac::Function> f) : function(f) {}
    ToArgumentsVisitor(std::shared_ptr<mtac::Function> f, bool take_address) : function(f), take_address(take_address) {}
    
    mutable std::shared_ptr<mtac::Function> function;
    bool take_address = false;

    result_type operator()(ast::Litteral& litteral) const {
        return {litteral.label, (int) litteral.value.size() - 2};
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

    result_type operator()(ast::BuiltinOperator& builtin) const {
        auto& value = builtin.Content->values[0];

        switch(builtin.Content->type){
            case ast::BuiltinType::SIZE:{
                ASSERT(boost::get<ast::VariableValue>(&value), "The size builtin can only be applied to variable");
                
                auto variable = boost::get<ast::VariableValue>(value).Content->var;

                if(variable->position().isGlobal()){
                    return {variable->type()->elements()};
                } else if(variable->position().isStack()){
                    return {variable->type()->elements()};
                } else if(variable->position().isParameter()){
                    auto t1 = function->context->newTemporary();

                    //The size of the array is at the address pointed by the variable
                    function->add(std::make_shared<mtac::Quadruple>(t1, variable, mtac::Operator::DOT, 0));

                    return {t1};
                }

                ASSERT_PATH_NOT_TAKEN("The variale is not of a valid type");
            }
            case ast::BuiltinType::LENGTH:
                return {visit(*this, value)[1]};
        }

        ASSERT_PATH_NOT_TAKEN("This builtin operator is not handled");
    }

    result_type operator()(ast::FunctionCall& call) const {
        auto type = call.Content->function->returnType;

        if(type == BOOL || type == INT || type == FLOAT || type->is_pointer()){
            auto t1 = function->context->new_temporary(type);

            execute_call(call, function, t1, {});

            return {t1};
        } else if(type == STRING){
            auto t1 = function->context->newTemporary();
            auto t2 = function->context->newTemporary();

            execute_call(call, function, t1, t2);

            return {t1, t2};
        }
        
        ASSERT_PATH_NOT_TAKEN("Unhandled function return type");
    }
    
    result_type operator()(ast::MemberFunctionCall& call) const {
        auto type = call.Content->function->returnType;

        if(type == BOOL || type == INT || type == FLOAT || type->is_pointer()){
            auto t1 = function->context->new_temporary(type);

            execute_member_call(call, function, t1, {});

            return {t1};
        } else if(type == STRING){
            auto t1 = function->context->newTemporary();
            auto t2 = function->context->newTemporary();

            execute_member_call(call, function, t1, t2);

            return {t1, t2};
        }
        
        ASSERT_PATH_NOT_TAKEN("Unhandled function return type");
    }

    result_type operator()(ast::Assignment& assignment) const {
        ASSERT(assignment.Content->op == ast::Operator::ASSIGN, "Compound assignment should be transformed into Assignment");

        assign(function, assignment);

        return visit(*this, assignment.Content->left_value);
    }

    result_type operator()(ast::Ternary& ternary) const {
        return compile_ternary(function, ternary);
    }

    result_type get_member(unsigned int offset, std::shared_ptr<const Type> member_type, std::shared_ptr<Variable> var) const {
        if(member_type == STRING){
            auto t1 = function->context->new_temporary(INT);
            auto t2 = function->context->new_temporary(INT);

            /*if(var->position().isParameter() && !var->type()->is_pointer()){
                function->add(std::make_shared<mtac::Quadruple>(t1, var, mtac::Operator::DOT, offset + getStringOffset(var)));
                function->add(std::make_shared<mtac::Quadruple>(t2, var, mtac::Operator::DOT, offset));
            } else {*/
                function->add(std::make_shared<mtac::Quadruple>(t1, var, mtac::Operator::DOT, offset));
                function->add(std::make_shared<mtac::Quadruple>(t2, var, mtac::Operator::DOT, offset + getStringOffset(var)));
            //}

            return {t1, t2};
        } else {
            auto temp = function->context->new_temporary(member_type);

            if(member_type == FLOAT){
                function->add(std::make_shared<mtac::Quadruple>(temp, var, mtac::Operator::FDOT, offset));
            } else if(member_type == INT || member_type == BOOL || member_type->is_pointer()){
                function->add(std::make_shared<mtac::Quadruple>(temp, var, mtac::Operator::DOT, offset));
            } else {
                ASSERT_PATH_NOT_TAKEN("Unhandled type");
            }

            return {temp};
        }
    }

    result_type operator()(ast::VariableValue& value) const {
        if(value.Content->memberNames.empty()){
            if(take_address){
                return {value.Content->var};
            }

            auto type = value.Content->var->type();

            //If it's a const, we just have to replace it by its constant value
            if(type->is_const()){
                auto val = value.Content->var->val();
                auto nc_type = type->non_const();

                if(nc_type == INT || nc_type == BOOL){
                    return {boost::get<int>(val)};
                } else if(nc_type == FLOAT){
                    return {boost::get<double>(val)};        
                } else if(nc_type == STRING){
                    auto value = boost::get<std::pair<std::string, int>>(val);

                    return {value.first, value.second};
                } 

                ASSERT_PATH_NOT_TAKEN("void is not a type");
            } else if(type->is_array() || type->is_pointer()){
                return {value.Content->var};
            } else {
                if(type == INT || type == BOOL || type == FLOAT){
                    return {value.Content->var};
                } else if(type == STRING){
                    auto temp = value.Content->context->newTemporary();
                    function->add(std::make_shared<mtac::Quadruple>(temp, value.Content->var, mtac::Operator::DOT, getStringOffset(value.Content->var)));

                    return {value.Content->var, temp};
                } else if(type->is_custom_type()) {
                    //If we are here, it means that we want to pass it by reference
                    return {value.Content->var};
                } else {
                    ASSERT_PATH_NOT_TAKEN("Unhandled type");
                }
            }
        } else {
            std::shared_ptr<const Type> member_type;
            unsigned int offset = 0;
            boost::tie(offset, member_type) = mtac::compute_member(value.variable(), value.Content->memberNames);

            if(take_address){
                auto temp = value.Content->context->new_temporary(INT);
                
                function->add(std::make_shared<mtac::Quadruple>(temp, value.Content->var, mtac::Operator::PDOT, offset));

                return {temp};
            }

            return get_member(offset, member_type, value.Content->var);
        }
    }

    result_type dereference_variable(std::shared_ptr<Variable> variable, std::shared_ptr<const Type> type) const {
        if(type == INT || type == BOOL){
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
            function->add(std::make_shared<mtac::Quadruple>(t2, variable, mtac::Operator::DOT, getStringOffset(variable)));

            return {t1, t2};
        } else {
            ASSERT_PATH_NOT_TAKEN("Unhandled type");
        }
    }

    template<typename Value>
    result_type dereference_sub(Value& value) const {
        auto values = visit_non_variant(*this, value);

        ASSERT(mtac::isVariable(values[0]), "The visitor should return a temporary variable");

        auto variable = boost::get<std::shared_ptr<Variable>>(values[0]);
        return dereference_variable(variable, visit_non_variant(ast::GetTypeVisitor(), value)->data_type());
    }
    
    result_type operator()(ast::DereferenceValue& dereference_value) const {
        if(auto* ptr = boost::get<ast::VariableValue>(&dereference_value.Content->ref)){
            auto& value = *ptr;

            if(value.Content->memberNames.empty()){
                auto type = value.variable()->type()->data_type();
                return dereference_variable(value.variable(), type);
            } else {
                return dereference_sub(value);
            }
        } else if(auto* ptr = boost::get<ast::ArrayValue>(&dereference_value.Content->ref)){
            return dereference_sub(*ptr);
        } else {
            ASSERT_PATH_NOT_TAKEN("Unhandled dereference left value type");
        }
    }

    result_type operator()(ast::PrefixOperation& operation) const {
        performPrefixOperation(operation, function);

        return {operation.Content->variable};
    }

    result_type operator()(ast::SuffixOperation& operation) const {
        return {performSuffixOperation(operation, function)};
    }

    result_type operator()(ast::ArrayValue& array) const {
        auto index = computeIndexOfArray(array.Content->var, array.Content->indexValue, function); 

        if(array.Content->memberNames.empty()){
            auto type = array.Content->var->type()->data_type();
            
            if(type == BOOL || type == INT || type == FLOAT || type->is_pointer()){
                auto temp = array.Content->context->new_temporary(type);
                function->add(std::make_shared<mtac::Quadruple>(temp, array.Content->var, mtac::Operator::DOT, index));
                return {temp};
            } else if (type == STRING){
                auto t1 = array.Content->context->newTemporary();
                function->add(std::make_shared<mtac::Quadruple>(t1, array.Content->var, mtac::Operator::DOT, index));

                auto t2 = array.Content->context->newTemporary();
                auto t3 = array.Content->context->newTemporary();

                //Assign the second part of the string
                function->add(std::make_shared<mtac::Quadruple>(t3, index, mtac::Operator::ADD, -INT->size()));
                function->add(std::make_shared<mtac::Quadruple>(t2, array.Content->var, mtac::Operator::DOT, t3));

                return {t1, t2};
            } else {
                ASSERT_PATH_NOT_TAKEN("void is not a variable");
            }
        } else {
            auto temp = array.Content->context->new_temporary(INT);
            function->add(std::make_shared<mtac::Quadruple>(temp, array.Content->var, mtac::Operator::PDOT, index));
            
            auto member_info = mtac::compute_member(array.Content->var, array.Content->memberNames);
            return get_member(member_info.first, member_info.second, temp);
        }
    }

    result_type operator()(ast::Expression& value) const {
        auto type = ast::GetTypeVisitor()(value);

        if(type == INT){
            return {performIntOperation(value, function)};
        } else if(type == FLOAT){
            return {performFloatOperation(value, function)};
        } else if(type == BOOL){
            return {performBoolOperation(value, function)};
        } else {
            auto t1 = function->context->newTemporary();
            auto t2 = function->context->newTemporary();

            performStringOperation(value, function, t1, t2);
            
            return {t1, t2};
        }
    }

    result_type operator()(ast::Unary& value) const {
        if(value.Content->op == ast::Operator::ADD){
            return visit(*this, value.Content->value);
        } else {
            mtac::Argument arg = moveToArgument(value.Content->value, function);

            auto type = visit(ast::GetTypeVisitor(), value.Content->value);
            auto t1 = function->context->new_temporary(type);

            if(type == FLOAT){
                function->add(std::make_shared<mtac::Quadruple>(t1, arg, mtac::Operator::FMINUS));
            } else {
                function->add(std::make_shared<mtac::Quadruple>(t1, arg, mtac::Operator::MINUS));
            }

            return {t1};
        }
    }
    
    result_type operator()(ast::Cast& cast) const {
        mtac::Argument arg = moveToArgument(cast.Content->value, function);
        
        auto srcType = visit(ast::GetTypeVisitor(), cast.Content->value);
        auto destType = visit(ast::TypeTransformer(), cast.Content->type);

        if(srcType != destType){
            auto t1 = function->context->new_temporary(destType);

            if(destType == FLOAT){
                function->add(std::make_shared<mtac::Quadruple>(t1, arg, mtac::Operator::I2F));
            } else if(destType == INT){
                function->add(std::make_shared<mtac::Quadruple>(t1, arg, mtac::Operator::F2I));
            }
            
            return {t1};
        }

        //If srcType == destType, there is nothing to do
        return {arg};
    }
};

struct AbstractVisitor : public boost::static_visitor<> {
    AbstractVisitor(std::shared_ptr<mtac::Function> f) : function(f) {}
    
    mutable std::shared_ptr<mtac::Function> function;
    
    virtual void intAssign(std::vector<mtac::Argument> arguments) const = 0;
    virtual void pointerAssign(std::vector<mtac::Argument> arguments) const = 0;
    virtual void floatAssign(std::vector<mtac::Argument> arguments) const = 0;
    virtual void stringAssign(std::vector<mtac::Argument> arguments) const = 0;
    
    template<typename T>
    void complexAssign(std::shared_ptr<const Type> type, T& value) const {
        if(type->is_pointer()){
            pointerAssign(ToArgumentsVisitor(function)(value));
        } else if(type == INT || type == BOOL){
            intAssign(ToArgumentsVisitor(function)(value));
        } else if(type == STRING){
            stringAssign(ToArgumentsVisitor(function)(value));
        } else if(type == FLOAT){
            floatAssign(ToArgumentsVisitor(function)(value));
        } else {
            ASSERT_PATH_NOT_TAKEN("Unhandled variable type");
        }
    }

    template<typename T>
    void operator()(T& value) const {
        complexAssign(ast::GetTypeVisitor()(value), value);
    }
};

struct AssignValueToVariable : public AbstractVisitor {
    AssignValueToVariable(std::shared_ptr<mtac::Function> f, std::shared_ptr<Variable> v) : AbstractVisitor(f), variable(v) {}
    AssignValueToVariable(std::shared_ptr<mtac::Function> f, std::shared_ptr<Variable> v, ast::Value& indexValue) : AbstractVisitor(f), variable(v), indexValue(indexValue) {}
    AssignValueToVariable(std::shared_ptr<mtac::Function> f, std::shared_ptr<Variable> v, unsigned int offset) : AbstractVisitor(f), variable(v), offset(offset) {}
    AssignValueToVariable(std::shared_ptr<mtac::Function> f, std::shared_ptr<Variable> v, unsigned int offset, std::shared_ptr<const Type> type) : AbstractVisitor(f), variable(v), type(type), offset(offset) {}
    
    std::shared_ptr<Variable> variable;
    std::shared_ptr<const Type> type;
    boost::optional<unsigned int> offset;
    boost::optional<ast::Value> indexValue;

    void intAssign(std::vector<mtac::Argument> arguments) const {
        if(offset){
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset, mtac::Operator::DOT_ASSIGN, arguments[0]));
        } else if(indexValue){
            auto index = computeIndexOfArray(variable, *indexValue, function); 
            function->add(std::make_shared<mtac::Quadruple>(variable, index, mtac::Operator::DOT_ASSIGN, arguments[0]));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(variable, arguments[0], mtac::Operator::ASSIGN));
        }
    }

    void pointerAssign(std::vector<mtac::Argument> arguments) const {
        if(offset){
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset, mtac::Operator::DOT_PASSIGN, arguments[0]));
        } else if(indexValue){
            auto index = computeIndexOfArray(variable, *indexValue, function); 
            function->add(std::make_shared<mtac::Quadruple>(variable, index, mtac::Operator::DOT_PASSIGN, arguments[0]));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(variable, arguments[0], mtac::Operator::PASSIGN));
        }
    }

    void floatAssign(std::vector<mtac::Argument> arguments) const {
        if(offset){
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset, mtac::Operator::DOT_FASSIGN, arguments[0]));
        } else if(indexValue){
            auto index = computeIndexOfArray(variable, *indexValue, function); 
            function->add(std::make_shared<mtac::Quadruple>(variable, index, mtac::Operator::DOT_FASSIGN, arguments[0]));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(variable, arguments[0], mtac::Operator::FASSIGN));
        }
    }

    void stringAssign(std::vector<mtac::Argument> arguments) const {
        if(offset){
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset, mtac::Operator::DOT_ASSIGN, arguments[0]));
            function->add(std::make_shared<mtac::Quadruple>(variable, *offset + getStringOffset(variable), mtac::Operator::DOT_ASSIGN, arguments[1]));
        } else if(indexValue){
            auto index = computeIndexOfArray(variable, *indexValue, function); 

            function->add(std::make_shared<mtac::Quadruple>(variable, index, mtac::Operator::DOT_ASSIGN, arguments[0]));

            auto temp1 = function->context->newTemporary();
            function->add(std::make_shared<mtac::Quadruple>(temp1, index, mtac::Operator::ADD, -INT->size()));
            function->add(std::make_shared<mtac::Quadruple>(variable, temp1, mtac::Operator::DOT_ASSIGN, arguments[1]));
        } else {
            function->add(std::make_shared<mtac::Quadruple>(variable, arguments[0], mtac::Operator::ASSIGN));
            function->add(std::make_shared<mtac::Quadruple>(variable, getStringOffset(variable), mtac::Operator::DOT_ASSIGN, arguments[1]));
        }
    }
    
    void operator()(ast::VariableValue& value) const {
        if(type){
            if(type->is_pointer()){
                pointerAssign(ToArgumentsVisitor(function, true)(value));
            } else {
                complexAssign(type, value);
            }
        } else {
            if(variable->type()->is_array() && variable->type()->data_type()->is_pointer()){
                pointerAssign(ToArgumentsVisitor(function)(value));
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
    DereferenceAssign(std::shared_ptr<mtac::Function> f, std::shared_ptr<Variable> v, unsigned int offset) : AbstractVisitor(f), variable(v), offset(offset) {}
    
    std::shared_ptr<Variable> variable;
    unsigned int offset;

    void intAssign(std::vector<mtac::Argument> arguments) const {
        if(offset == 0){
            function->add(std::make_shared<mtac::Quadruple>(variable, 0, mtac::Operator::DOT_ASSIGN, arguments[0]));
        } else {
            auto temp = function->context->new_temporary(INT);

            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::DOT, offset));
            function->add(std::make_shared<mtac::Quadruple>(temp, 0, mtac::Operator::DOT_ASSIGN, arguments[0]));
        }
    }
    
    void pointerAssign(std::vector<mtac::Argument> arguments) const {
        if(offset == 0){
            function->add(std::make_shared<mtac::Quadruple>(variable, 0, mtac::Operator::DOT_PASSIGN, arguments[0]));
        } else {
            auto temp = function->context->new_temporary(INT);

            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::DOT, offset));
            function->add(std::make_shared<mtac::Quadruple>(temp, 0, mtac::Operator::DOT_PASSIGN, arguments[0]));
        }
    }
    
    void floatAssign(std::vector<mtac::Argument> arguments) const {
        if(offset == 0){
            function->add(std::make_shared<mtac::Quadruple>(variable, 0, mtac::Operator::DOT_FASSIGN, arguments[0]));
        } else {
            auto temp = function->context->new_temporary(INT);

            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::FDOT, offset));
            function->add(std::make_shared<mtac::Quadruple>(temp, 0, mtac::Operator::DOT_FASSIGN, arguments[0]));
        }
    }

    void stringAssign(std::vector<mtac::Argument> arguments) const {
        if(offset == 0){
            function->add(std::make_shared<mtac::Quadruple>(variable, 0, mtac::Operator::DOT_ASSIGN, arguments[0]));
            function->add(std::make_shared<mtac::Quadruple>(variable, getStringOffset(variable), mtac::Operator::DOT_ASSIGN, arguments[1]));
        } else {
            auto temp = function->context->new_temporary(INT);
            
            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::DOT, offset));

            function->add(std::make_shared<mtac::Quadruple>(temp, offset, mtac::Operator::DOT_ASSIGN, arguments[0]));
            function->add(std::make_shared<mtac::Quadruple>(temp, offset + getStringOffset(variable), mtac::Operator::DOT_ASSIGN, arguments[1]));
        }
    }
};

void assign(std::shared_ptr<mtac::Function> function, ast::Assignment& assignment){
    if(auto* ptr = boost::get<ast::VariableValue>(&assignment.Content->left_value)){
        auto left = *ptr;
        auto variable = left.Content->var;

        if(left.Content->memberNames.empty()){
            visit(AssignValueToVariable(function, variable), assignment.Content->value);
        } else {
            unsigned int offset = 0;
            std::shared_ptr<const Type> member_type;
            boost::tie(offset, member_type) = mtac::compute_member(variable, left.Content->memberNames);

            visit(AssignValueToVariable(function, variable, offset, member_type), assignment.Content->value);
        }
    } else if(auto* ptr = boost::get<ast::ArrayValue>(&assignment.Content->left_value)){
        auto left = *ptr;
        auto variable = left.Content->var;

        if(left.Content->memberNames.empty()){
            visit(AssignValueToVariable(function, variable, left.Content->indexValue), assignment.Content->value);
        } else {
            auto index = computeIndexOfArray(variable, left.Content->indexValue, function); 
            
            auto temp = left.Content->context->new_temporary(INT);
            function->add(std::make_shared<mtac::Quadruple>(temp, variable, mtac::Operator::PDOT, index));
            
            unsigned int offset = 0;
            std::shared_ptr<const Type> member_type;
            boost::tie(offset, member_type) = mtac::compute_member(variable, left.Content->memberNames);
            
            visit(AssignValueToVariable(function, temp, offset, member_type), assignment.Content->value);
        }
    } else if(auto* ptr = boost::get<ast::DereferenceValue>(&assignment.Content->left_value)){
        if(auto* var_ptr = boost::get<ast::VariableValue>(&(*ptr).Content->ref)){
            auto left = *var_ptr;
        
            auto variable = left.Content->var;

            if(left.Content->memberNames.empty()){
                visit(DereferenceAssign(function, variable, 0), assignment.Content->value);
            } else {
                unsigned int offset = mtac::compute_member_offset(variable, left.Content->memberNames);

                visit(DereferenceAssign(function, variable, offset), assignment.Content->value);
            }
        } else if(auto* array_ptr = boost::get<ast::ArrayValue>(&(*ptr).Content->ref)){
            auto left = *array_ptr;

            //As the array hold pointers, the visitor will return a temporary
            auto values = ToArgumentsVisitor(function)(left);

            ASSERT(mtac::isVariable(values[0]), "The visitor should return a temporary variable");

            auto variable = boost::get<std::shared_ptr<Variable>>(values[0]);

            visit(DereferenceAssign(function, variable, 0), assignment.Content->value);
        }
    }
}

struct JumpIfFalseVisitor : public boost::static_visitor<> {
    JumpIfFalseVisitor(std::shared_ptr<mtac::Function> f, const std::string& l) : function(f), label(l) {}
    
    mutable std::shared_ptr<mtac::Function> function;
    std::string label;
   
    void operator()(ast::Expression& value) const ;
    
    template<typename T>
    void operator()(T& value) const {
        auto argument = ToArgumentsVisitor(function)(value)[0];

        function->add(std::make_shared<mtac::IfFalse>(argument, label));
    }
};

template<typename Control>
void compare(ast::Expression& value, ast::Operator op, std::shared_ptr<mtac::Function> function, const std::string& label){
    ASSERT(value.Content->operations.size() == 1, "Relational operations cannot be chained");

    auto left = moveToArgument(value.Content->first, function);
    auto right = moveToArgument(value.Content->operations[0].get<1>(), function);

    auto typeLeft = visit(ast::GetTypeVisitor(), value.Content->first);

    if(typeLeft == INT){
        function->add(std::make_shared<Control>(mtac::toBinaryOperator(op), left, right, label));
    } else if(typeLeft == FLOAT){
        function->add(std::make_shared<Control>(mtac::toFloatBinaryOperator(op), left, right, label));
    } 
}

struct JumpIfTrueVisitor : public boost::static_visitor<> {
    JumpIfTrueVisitor(std::shared_ptr<mtac::Function> f, const std::string& l) : function(f), label(l) {}
    
    mutable std::shared_ptr<mtac::Function> function;
    std::string label;
   
    void operator()(ast::Expression& value) const {
        auto op = value.Content->operations[0].get<0>();

        //Logical and operators (&&)
        if(op == ast::Operator::AND){
            std::string codeLabel = newLabel();

            visit(JumpIfFalseVisitor(function, codeLabel), value.Content->first);

            for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
                if(i == value.Content->operations.size() - 1){
                    visit(*this, value.Content->operations[i].get<1>());   
                } else {
                    visit(JumpIfFalseVisitor(function, codeLabel), value.Content->operations[i].get<1>());
                }
            }

            function->add(codeLabel);
        } 
        //Logical or operators (||)
        else if(op == ast::Operator::OR){
            visit(*this, value.Content->first);

            for(auto& operation : value.Content->operations){
                visit(*this, operation.get<1>());
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
        auto argument = ToArgumentsVisitor(function)(value)[0];

        function->add(std::make_shared<mtac::If>(argument, label));
    }
};

void JumpIfFalseVisitor::operator()(ast::Expression& value) const {
    auto op = value.Content->operations[0].get<0>();

    //Logical and operators (&&)
    if(op == ast::Operator::AND){
        visit(*this, value.Content->first);

        for(auto& operation : value.Content->operations){
            visit(*this, operation.get<1>());
        }
    } 
    //Logical or operators (||)
    else if(op == ast::Operator::OR){
        std::string codeLabel = newLabel();

        visit(JumpIfTrueVisitor(function, codeLabel), value.Content->first);

        for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
            if(i == value.Content->operations.size() - 1){
                visit(*this, value.Content->operations[i].get<1>());   
            } else {
                visit(JumpIfTrueVisitor(function, codeLabel), value.Content->operations[i].get<1>());
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

std::vector<mtac::Argument> compile_ternary(std::shared_ptr<mtac::Function> function, ast::Ternary& ternary){
    auto type = visit_non_variant(ast::GetTypeVisitor(), ternary);

    auto falseLabel = newLabel();
    auto endLabel = newLabel();

    if(type == INT || type == BOOL || type == FLOAT){
        auto t1 = function->context->new_temporary(type);

        visit(JumpIfFalseVisitor(function, falseLabel), ternary.Content->condition); 
        visit(AssignValueToVariable(function, t1), ternary.Content->true_value);
        function->add(std::make_shared<mtac::Goto>(endLabel));
        
        function->add(falseLabel);
        visit(AssignValueToVariable(function, t1), ternary.Content->false_value);
        
        function->add(endLabel);

        return {t1};
    } else if(type == STRING){
        auto t1 = function->context->newTemporary();
        auto t2 = function->context->newTemporary();
        
        visit(JumpIfFalseVisitor(function, falseLabel), ternary.Content->condition); 
        auto args = visit(ToArgumentsVisitor(function), ternary.Content->true_value);
        function->add(std::make_shared<mtac::Quadruple>(t1, args[0], mtac::Operator::ASSIGN));  
        function->add(std::make_shared<mtac::Quadruple>(t2, args[1], mtac::Operator::ASSIGN));  

        function->add(std::make_shared<mtac::Goto>(endLabel));
        
        function->add(falseLabel);
        args = visit(ToArgumentsVisitor(function), ternary.Content->false_value);
        function->add(std::make_shared<mtac::Quadruple>(t1, args[0], mtac::Operator::ASSIGN));  
        function->add(std::make_shared<mtac::Quadruple>(t2, args[1], mtac::Operator::ASSIGN));  
        
        function->add(endLabel);
        
        return {t1, t2};
    }

    ASSERT_PATH_NOT_TAKEN("Unhandled ternary type");

    return {};
}

void performStringOperation(ast::Expression& value, std::shared_ptr<mtac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2){
    ASSERT(value.Content->operations.size() > 0, "Expression with no operation should have been transformed");

    std::vector<mtac::Argument> arguments;

    auto first = visit(ToArgumentsVisitor(function), value.Content->first);
    arguments.insert(arguments.end(), first.begin(), first.end());

    //Perfom all the additions
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        auto second = visit(ToArgumentsVisitor(function), operation.get<1>());
        arguments.insert(arguments.end(), second.begin(), second.end());
        
        for(auto& arg : arguments){
            function->add(std::make_shared<mtac::Param>(arg));   
        }

        arguments.clear();
        
        symbols.addReference("_F6concatSS");

        if(i == value.Content->operations.size() - 1){
            function->add(std::make_shared<mtac::Call>("_F6concatSS", symbols.getFunction("_F6concatSS"), v1, v2)); 
        } else {
            auto t1 = function->context->newTemporary();
            auto t2 = function->context->newTemporary();
            
            function->add(std::make_shared<mtac::Call>("_F6concatSS", symbols.getFunction("_F6concatSS"), t1, t2)); 
          
            arguments.push_back(t1);
            arguments.push_back(t2);
        }
    }
}

class CompilerVisitor : public boost::static_visitor<> {
    private:
        std::shared_ptr<StringPool> pool;
        std::shared_ptr<mtac::Program> program;
        std::shared_ptr<mtac::Function> function;
    
    public:
        CompilerVisitor(std::shared_ptr<StringPool> p, std::shared_ptr<mtac::Program> mtacProgram) : pool(p), program(mtacProgram){}

        //No code is generated for these nodes
        AUTO_IGNORE_GLOBAL_VARIABLE_DECLARATION()
        AUTO_IGNORE_GLOBAL_ARRAY_DECLARATION()
        AUTO_IGNORE_ARRAY_DECLARATION()
        AUTO_IGNORE_IMPORT()
        AUTO_IGNORE_STANDARD_IMPORT()
       
        void operator()(ast::Assignment& assignment){
            ASSERT(assignment.Content->op == ast::Operator::ASSIGN, "Compound assignment should be transformed into Assignment");

            assign(function, assignment);
        }
        
        void operator()(ast::SourceFile& p){
            program->context = p.Content->context;

            visit_each(*this, p.Content->blocks);
        }
        
        void operator()(ast::Struct& p){
            visit_each_non_variant(*this, p.Content->functions);
        }

        void operator()(ast::FunctionDeclaration& f){
            function = std::make_shared<mtac::Function>(f.Content->context, f.Content->mangledName);
            function->definition = symbols.getFunction(f.Content->mangledName);

            visit_each(*this, f.Content->instructions);

            program->functions.push_back(function);
        }

        void operator()(ast::If& if_){
            if (if_.Content->elseIfs.empty()) {
                std::string endLabel = newLabel();

                visit(JumpIfFalseVisitor(function, endLabel), if_.Content->condition);

                visit_each(*this, if_.Content->instructions);

                if (if_.Content->else_) {
                    std::string elseLabel = newLabel();

                    function->add(std::make_shared<mtac::Goto>(elseLabel));

                    function->add(endLabel);

                    visit_each(*this, (*if_.Content->else_).instructions);

                    function->add(elseLabel);
                } else {
                    function->add(endLabel);
                }
            } else {
                std::string end = newLabel();
                std::string next = newLabel();

                visit(JumpIfFalseVisitor(function, next), if_.Content->condition);

                visit_each(*this, if_.Content->instructions);

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

                    function->add(std::make_shared<mtac::Goto>(end));
                }

                if (if_.Content->else_) {
                    function->add(next);

                    visit_each(*this, (*if_.Content->else_).instructions);
                }

                function->add(end);
            }
        }

        void operator()(ast::VariableDeclaration& declaration){
            if(declaration.Content->value){
                auto var = declaration.Content->context->getVariable(declaration.Content->variableName);
                
                if(!var->type()->is_const()){
                    visit(AssignValueToVariable(function, var), *declaration.Content->value);
                }
            }
        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;
            
            auto t1 = swap.Content->context->newTemporary();

            if(lhs_var->type() == INT || lhs_var->type() == BOOL || lhs_var->type() == STRING){
                function->add(std::make_shared<mtac::Quadruple>(t1, rhs_var, mtac::Operator::ASSIGN));  
                function->add(std::make_shared<mtac::Quadruple>(rhs_var, lhs_var, mtac::Operator::ASSIGN));  
                function->add(std::make_shared<mtac::Quadruple>(lhs_var, t1, mtac::Operator::ASSIGN));  
                
                if(lhs_var->type() == STRING){
                    auto t2 = swap.Content->context->newTemporary();

                    //t1 = 4(b)
                    function->add(std::make_shared<mtac::Quadruple>(t1, rhs_var, mtac::Operator::DOT, getStringOffset(rhs_var)));  
                    //t2 = 4(a)
                    function->add(std::make_shared<mtac::Quadruple>(t2, lhs_var, mtac::Operator::DOT, getStringOffset(lhs_var)));  
                    //4(b) = t2
                    function->add(std::make_shared<mtac::Quadruple>(rhs_var, getStringOffset(rhs_var), mtac::Operator::DOT_ASSIGN, t2));  
                    //4(a) = t1
                    function->add(std::make_shared<mtac::Quadruple>(lhs_var, getStringOffset(lhs_var), mtac::Operator::DOT_ASSIGN, t1));  
                }
            } else {
                ASSERT_PATH_NOT_TAKEN("Unhandled variable type");
            }
        }

        void operator()(ast::SuffixOperation& operation){
            //As we don't need the return value, we can make it prefix
            performPrefixOperation(operation, function);
        }
        
        void operator()(ast::PrefixOperation& operation){
            performPrefixOperation(operation, function);
        }

        void operator()(ast::DoWhile& while_){
            std::string startLabel = newLabel();

            function->add(startLabel);

            visit_each(*this, while_.Content->instructions);

            visit(JumpIfTrueVisitor(function, startLabel), while_.Content->condition);
        }

        void operator()(ast::FunctionCall& functionCall){
            execute_call(functionCall, function, {}, {});
        }
        
        void operator()(ast::MemberFunctionCall& functionCall){
            execute_member_call(functionCall, function, {}, {});
        }

        void operator()(ast::Return& return_){
            auto arguments = visit(ToArgumentsVisitor(function), return_.Content->value);

            if(arguments.size() == 1){
                function->add(std::make_shared<mtac::Quadruple>(mtac::Operator::RETURN, arguments[0]));
            } else if(arguments.size() == 2){
                function->add(std::make_shared<mtac::Quadruple>(mtac::Operator::RETURN, arguments[0], arguments[1]));
            } else {
                ASSERT_PATH_NOT_TAKEN("Unhandled arguments size");
            }   
        }

        template<typename T>
        void operator()(T&){
            ASSERT_PATH_NOT_TAKEN("This element should have been transformed"); 
        }
};

mtac::Argument moveToArgument(ast::Value& value, std::shared_ptr<mtac::Function> function){
    return visit(ToArgumentsVisitor(function), value)[0];
}
    
void push_struct_member(ast::VariableValue& memberValue, std::shared_ptr<const Type> type, std::shared_ptr<mtac::Function> function, boost::variant<std::shared_ptr<Variable>, std::string> param, std::shared_ptr<Function> definition){
    auto struct_name = type->type();
    auto struct_type = symbols.get_struct(struct_name);

    for(auto& member : boost::adaptors::reverse(struct_type->members)){
        auto member_type = member->type;

        memberValue.Content->memberNames.push_back(member->name);

        if(member_type->is_custom_type()){
            push_struct_member(memberValue, member_type, function, param, definition);
        } else {
            auto member_values = ToArgumentsVisitor(function)(memberValue);
            std::reverse(member_values.begin(), member_values.end());

            for(auto& v : member_values){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param)){
                    function->add(std::make_shared<mtac::Param>(v, *ptr, definition));
                } else if(auto* ptr = boost::get<std::string>(&param)){
                    function->add(std::make_shared<mtac::Param>(v, *ptr, definition));
                }
            }
        }

        memberValue.Content->memberNames.pop_back();
    }
}

void push_struct(std::shared_ptr<mtac::Function> function, boost::variant<std::shared_ptr<Variable>, std::string> param, std::shared_ptr<Function> definition, ast::VariableValue& value){
    auto var = value.Content->var;
    auto context = value.Content->context;

    auto struct_name = var->type()->type();
    auto struct_type = symbols.get_struct(struct_name);

    for(auto& member : boost::adaptors::reverse(struct_type->members)){
        auto type = member->type;

        ast::VariableValue memberValue;
        memberValue.Content->context = context;
        memberValue.Content->variableName = var->name();
        memberValue.Content->var = var;
        memberValue.Content->memberNames = {member->name};

        if(type->is_custom_type()){
            push_struct_member(memberValue, type, function, param, definition);
        } else {
            auto member_values = ToArgumentsVisitor(function)(memberValue);
            std::reverse(member_values.begin(), member_values.end());

            for(auto& v : member_values){
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param)){
                    function->add(std::make_shared<mtac::Param>(v, *ptr, definition));
                } else if(auto* ptr = boost::get<std::string>(&param)){
                    function->add(std::make_shared<mtac::Param>(v, *ptr, definition));
                }
            }
        }
    }
}

template<typename Call>
void pass_arguments(std::shared_ptr<mtac::Function> function, std::shared_ptr<eddic::Function> definition, Call& functionCall){
    auto context = definition->context;
    
    auto values = functionCall.Content->values;
    std::reverse(values.begin(), values.end());

    //If it's a standard function, there are no context
    if(!context){
        auto parameters = definition->parameters;
        int i = parameters.size()-1;

        for(auto& first : values){
            auto param = parameters[i--].name; 
            
            auto args = visit(ToArgumentsVisitor(function), first);
            for(auto& arg : boost::adaptors::reverse(args)){
                function->add(std::make_shared<mtac::Param>(arg, param, definition));   
            }
        }
    } else {
        auto parameters = definition->parameters;
        int i = parameters.size()-1;

        for(auto& first : values){
            std::shared_ptr<Variable> param = context->getVariable(parameters[i--].name);

            if(auto* ptr = boost::get<ast::VariableValue>(&first)){
                auto type = (*ptr).Content->var->type();
                if((*ptr).Content->memberNames.empty() && type->is_custom_type() && !param->type()->is_pointer()){
                    push_struct(function, param, definition, *ptr);
                    continue;
                }

                if(!(*ptr).Content->memberNames.empty() && type->is_custom_type() && param->type()->is_pointer()){
                    auto mtac_param = std::make_shared<mtac::Param>((*ptr).Content->var, param, definition);

                    mtac_param->address = true;
                    mtac_param->memberNames = (*ptr).Content->memberNames;

                    function->add(mtac_param);

                    continue;
                }
            } 

            auto args = visit(ToArgumentsVisitor(function, param->type()->is_pointer()), first);
            for(auto& arg : boost::adaptors::reverse(args)){
                auto mtac_param = std::make_shared<mtac::Param>(arg, param, definition);
                mtac_param->address = param->type()->is_pointer();

                function->add(mtac_param);   
            }
        }
    }
}

void execute_call(ast::FunctionCall& functionCall, std::shared_ptr<mtac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_){
    std::shared_ptr<eddic::Function> definition;
    if(functionCall.Content->mangled_name.empty()){
        definition = symbols.getFunction(mangle(functionCall.Content->functionName, functionCall.Content->values));
    } else if(functionCall.Content->function){
        definition = functionCall.Content->function;
    } else {
        definition = symbols.getFunction(functionCall.Content->mangled_name);
    }

    ASSERT(definition, "All the functions should be in the function table");

    pass_arguments(function, definition, functionCall);

    function->add(std::make_shared<mtac::Call>(definition->mangledName, definition, return_, return2_));
}

void execute_member_call(ast::MemberFunctionCall& functionCall, std::shared_ptr<mtac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_){
    auto var = functionCall.Content->context->getVariable(functionCall.Content->object_name);

    auto definition = functionCall.Content->function;

    ASSERT(definition, "All the member functions should be in the function table");

    //Pass all normal arguments
    pass_arguments(function, definition, functionCall);
                
    //Pass the address of the object to the member function
    auto mtac_param = std::make_shared<mtac::Param>(var, definition->context->getVariable(definition->parameters[0].name), definition);
    mtac_param->address = true;
    function->add(mtac_param);   

    //Call the function
    function->add(std::make_shared<mtac::Call>(definition->mangledName, definition, return_, return2_));
}

std::shared_ptr<Variable> performBoolOperation(ast::Expression& value, std::shared_ptr<mtac::Function> function){
    auto t1 = function->context->newTemporary(); 
   
    //The first operator defines the kind of operation 
    auto op = value.Content->operations[0].get<0>();

    //Logical and operators (&&)
    if(op == ast::Operator::AND){
        auto falseLabel = newLabel();
        auto endLabel = newLabel();

        visit(JumpIfFalseVisitor(function, falseLabel), value.Content->first);

        for(auto& operation : value.Content->operations){
            visit(JumpIfFalseVisitor(function, falseLabel), operation.get<1>());
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
            visit(JumpIfTrueVisitor(function, trueLabel), operation.get<1>());
        }

        function->add(std::make_shared<mtac::Quadruple>(t1, 0, mtac::Operator::ASSIGN));
        function->add(std::make_shared<mtac::Goto>(endLabel));

        function->add(trueLabel);
        function->add(std::make_shared<mtac::Quadruple>(t1, 1, mtac::Operator::ASSIGN));

        function->add(endLabel);
    }
    //Relational operators 
    else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
        ASSERT(value.Content->operations.size() == 1, "Relational operations cannot be chained");

        auto left = moveToArgument(value.Content->first, function);
        auto right = moveToArgument(value.Content->operations[0].get<1>(), function);
        
        auto typeLeft = visit(ast::GetTypeVisitor(), value.Content->first);
        if(typeLeft == INT){
            function->add(std::make_shared<mtac::Quadruple>(t1, left, mtac::toRelationalOperator(op), right));
        } else if(typeLeft == FLOAT){
            function->add(std::make_shared<mtac::Quadruple>(t1, left, mtac::toFloatRelationalOperator(op), right));
        }
    } 
    
    return t1;
}

} //end of anonymous namespace

void mtac::Compiler::compile(ast::SourceFile& program, std::shared_ptr<StringPool> pool, std::shared_ptr<mtac::Program> mtacProgram) const {
    PerfsTimer timer("MTAC Compilation");

    CompilerVisitor visitor(pool, mtacProgram);
    visitor(program);
}
