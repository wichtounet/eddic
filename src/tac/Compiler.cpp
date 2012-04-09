//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "assert.hpp"
#include "VisitorUtils.hpp"
#include "Variable.hpp"
#include "SymbolTable.hpp"
#include "SemanticalException.hpp"
#include "FunctionContext.hpp"
#include "mangling.hpp"
#include "Labels.hpp"

#include "tac/Compiler.hpp"
#include "tac/Program.hpp"
#include "tac/IsSingleArgumentVisitor.hpp"
#include "tac/IsParamSafeVisitor.hpp"

#include "ast/SourceFile.hpp"
#include "ast/GetTypeVisitor.hpp"
#include "ast/TypeTransformer.hpp"

using namespace eddic;

SymbolTable* symbols;

namespace {

void performStringOperation(ast::Expression& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2);
void executeCall(ast::FunctionCall& functionCall, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);
tac::Argument moveToArgument(ast::Value& value, std::shared_ptr<tac::Function> function);

std::shared_ptr<Variable> performOperation(ast::Expression& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> t1, tac::Operator f(ast::Operator)){
    ASSERT(value.Content->operations.size() > 0, "Operations with no operation should have been transformed before");

    tac::Argument left = moveToArgument(value.Content->first, function);
    tac::Argument right;

    //Apply all the operations in chain
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        right = moveToArgument(operation.get<1>(), function);
       
        if (i == 0){
            function->add(std::make_shared<tac::Quadruple>(t1, left, f(operation.get<0>()), right));
        } else {
            function->add(std::make_shared<tac::Quadruple>(t1, t1, f(operation.get<0>()), right));
        }
    }

    return t1;
}

std::shared_ptr<Variable> performIntOperation(ast::Expression& value, std::shared_ptr<tac::Function> function){
    return performOperation(value, function, function->context->newTemporary(), &tac::toOperator);
}

std::shared_ptr<Variable> performFloatOperation(ast::Expression& value, std::shared_ptr<tac::Function> function){
    return performOperation(value, function, function->context->newFloatTemporary(), &tac::toFloatOperator);
}

std::shared_ptr<Variable> performBoolOperation(ast::Expression& value, std::shared_ptr<tac::Function> function);

tac::Argument computeIndexOfArray(std::shared_ptr<Variable> array, tac::Argument index, std::shared_ptr<tac::Function> function){
    auto temp = function->context->newTemporary();
    auto position = array->position();

    if(position.isGlobal()){
        function->add(std::make_shared<tac::Quadruple>(temp, index, tac::Operator::MUL, -1 * size(array->type().base())));

        //Compute the offset manually to avoid having ADD then SUB
        //TODO Find a way to make that optimization in the TAC Optimizer
        int offset = size(array->type().base()) * array->type().size();
        offset -= size(BaseType::INT);

        function->add(std::make_shared<tac::Quadruple>(temp, temp, tac::Operator::ADD, offset));
    } else if(position.isStack()){
        function->add(std::make_shared<tac::Quadruple>(temp, index, tac::Operator::MUL, size(array->type().base())));
        function->add(std::make_shared<tac::Quadruple>(temp, temp, tac::Operator::ADD, size(BaseType::INT)));
        function->add(std::make_shared<tac::Quadruple>(temp, temp, tac::Operator::MUL, -1));
    } else if(position.isParameter()){
        function->add(std::make_shared<tac::Quadruple>(temp, index, tac::Operator::MUL, size(array->type().base())));
        function->add(std::make_shared<tac::Quadruple>(temp, temp, tac::Operator::ADD, size(BaseType::INT)));
        function->add(std::make_shared<tac::Quadruple>(temp, temp, tac::Operator::MUL, -1));
    }
   
    return temp;
}

tac::Argument computeIndexOfArray(std::shared_ptr<Variable> array, ast::Value& indexValue, std::shared_ptr<tac::Function> function){
    tac::Argument index = moveToArgument(indexValue, function);

    return computeIndexOfArray(array, index, function);
}

std::shared_ptr<Variable> computeLengthOfArray(std::shared_ptr<Variable> array, std::shared_ptr<tac::Function> function){
    auto t1 = function->context->newTemporary();
    
    auto position = array->position();
    if(position.isGlobal() || position.isStack()){
        function->add(std::make_shared<tac::Quadruple>(t1, array->type().size(), tac::Operator::ASSIGN));
    } else if(position.isParameter()){
        function->add(std::make_shared<tac::Quadruple>(t1, array, tac::Operator::ARRAY, 0));
    }

    return t1;
}

int getStringOffset(std::shared_ptr<Variable> variable){
    return variable->position().isGlobal() ? size(BaseType::INT) : -size(BaseType::INT);
}

void assign(std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> variable, ast::Value& value);
    
template<typename Operation>
void performPrefixOperation(const Operation& operation, std::shared_ptr<tac::Function> function){
    auto var = operation.Content->variable;

    if(operation.Content->op == ast::Operator::INC){
        if(var->type() == BaseType::FLOAT){
            function->add(std::make_shared<tac::Quadruple>(var, var, tac::Operator::FADD, 1.0));
        } else {
            function->add(std::make_shared<tac::Quadruple>(var, var, tac::Operator::ADD, 1));
        }
    } else if(operation.Content->op == ast::Operator::DEC){
        if(var->type() == BaseType::FLOAT){
            function->add(std::make_shared<tac::Quadruple>(var, var, tac::Operator::FSUB, 1.0));
        } else {
            function->add(std::make_shared<tac::Quadruple>(var, var, tac::Operator::SUB, 1));
        }
    }
}

template<typename Operation>
std::shared_ptr<Variable> performSuffixOperation(const Operation& operation, std::shared_ptr<tac::Function> function){
    auto var = operation.Content->variable;

    if(var->type() == BaseType::FLOAT){
        auto temp = operation.Content->context->newFloatTemporary();

        function->add(std::make_shared<tac::Quadruple>(temp, var, tac::Operator::FASSIGN));

        if(operation.Content->op == ast::Operator::INC){
            function->add(std::make_shared<tac::Quadruple>(var, var, tac::Operator::FADD, 1.0));
        } else if(operation.Content->op == ast::Operator::DEC){
            function->add(std::make_shared<tac::Quadruple>(var, var, tac::Operator::FSUB, 1.0));
        }

        return temp;
    } else {
        auto temp = operation.Content->context->newTemporary();

        function->add(std::make_shared<tac::Quadruple>(temp, var, tac::Operator::ASSIGN));

        if(operation.Content->op == ast::Operator::INC){
            function->add(std::make_shared<tac::Quadruple>(var, var, tac::Operator::ADD, 1));
        } else if(operation.Content->op == ast::Operator::DEC){
            function->add(std::make_shared<tac::Quadruple>(var, var, tac::Operator::SUB, 1));
        }

        return temp;
    }
}

struct ToArgumentsVisitor : public boost::static_visitor<std::vector<tac::Argument>> {
    ToArgumentsVisitor(std::shared_ptr<tac::Function> f) : function(f) {}
    
    mutable std::shared_ptr<tac::Function> function;

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

    result_type operator()(ast::BuiltinOperator& builtin) const {
        auto& value = builtin.Content->values[0];

        switch(builtin.Content->type){
            case ast::BuiltinType::SIZE:{
                ASSERT(boost::get<ast::VariableValue>(&value), "The size builtin can only be applied to variable");
                
                auto variable = boost::get<ast::VariableValue>(value).Content->var;

                if(variable->position().isGlobal()){
                    return {variable->type().size()};
                } else if(variable->position().isStack()){
                    return {variable->type().size()};
                } else if(variable->position().isParameter()){
                    auto t1 = function->context->newTemporary();

                    //The size of the array is at the address pointed by the variable
                    function->add(std::make_shared<tac::Quadruple>(t1, variable, tac::Operator::DOT, 0));

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
        Type type = call.Content->function->returnType;

        switch(type.base()){
            case BaseType::BOOL:
            case BaseType::INT:{
                auto t1 = function->context->newTemporary();

                executeCall(call, function, t1, {});

                return {t1};
            }
            case BaseType::FLOAT:{
                auto t1 = function->context->newFloatTemporary();

                executeCall(call, function, t1, {});

                return {t1};
            }
            case BaseType::STRING:{
                auto t1 = function->context->newTemporary();
                auto t2 = function->context->newTemporary();

                executeCall(call, function, t1, t2);

                return {t1, t2};
            }
            default:
                throw SemanticalException("This function doesn't return anything");   
        }
    }

    result_type operator()(ast::Assignment& assignment) const {
        auto variable = assignment.Content->context->getVariable(assignment.Content->variableName);

        assign(function, variable, assignment.Content->value);

        return {variable};
    }
            
    result_type operator()(ast::VariableValue& value) const {
        auto type = value.Content->var->type();

        //If it's a const, we just have to replace it by its constant value
        if(type.isConst()){
           auto val = value.Content->var->val();

           switch(type.base()){
               case BaseType::INT:
               case BaseType::BOOL:
                   return {boost::get<int>(val)};
               case BaseType::FLOAT:
                   return {boost::get<double>(val)};        
               case BaseType::STRING:{
                     auto value = boost::get<std::pair<std::string, int>>(val);

                     return {value.first, value.second};
                }
                default:
                     ASSERT_PATH_NOT_TAKEN("void is not a type");
           }
        } else if(type.isArray()){
            return {value.Content->var};
        } else {
            if(type == BaseType::INT || type == BaseType::BOOL || type == BaseType::FLOAT){
                return {value.Content->var};
            } else {
                auto temp = value.Content->context->newTemporary();
                function->add(std::make_shared<tac::Quadruple>(temp, value.Content->var, tac::Operator::DOT, getStringOffset(value.Content->var)));

                return {value.Content->var, temp};
            }
        }
    }

    result_type operator()(ast::PrefixOperation& operation) const {
        performPrefixOperation(operation, function);

        return {operation.Content->variable};
    }

    result_type operator()(ast::SuffixOperation& operation) const {
        return {performSuffixOperation(operation, function)};
    }

    result_type operator()(ast::StructValue& value) const {
        auto struct_name = value.Content->variable->type().type();
        auto struct_type = symbols->get_struct(struct_name);
        auto member_type = (*struct_type)[value.Content->memberName].type;
        auto offset = symbols->member_offset(struct_type, value.Content->memberName);

        auto temp = value.Content->context->new_temporary(member_type);
        
        function->add(std::make_shared<tac::Quadruple>(temp, value.Content->variable, tac::Operator::DOT, offset));

        return {temp};
    }

    result_type operator()(ast::ArrayValue& array) const {
        auto index = computeIndexOfArray(array.Content->var, array.Content->indexValue, function); 

        switch(array.Content->var->type().base()){
            case BaseType::BOOL:
            case BaseType::INT:
            case BaseType::FLOAT: {
                auto temp = array.Content->context->new_temporary(array.Content->var->type());
                function->add(std::make_shared<tac::Quadruple>(temp, array.Content->var, tac::Operator::ARRAY, index));

                return {temp};

            }   
            case BaseType::STRING: {
                auto t1 = array.Content->context->newTemporary();
                function->add(std::make_shared<tac::Quadruple>(t1, array.Content->var, tac::Operator::ARRAY, index));
                    
                auto t2 = array.Content->context->newTemporary();
                auto t3 = array.Content->context->newTemporary();
                
                //Assign the second part of the string
                function->add(std::make_shared<tac::Quadruple>(t3, index, tac::Operator::ADD, -size(BaseType::INT)));
                function->add(std::make_shared<tac::Quadruple>(t2, array.Content->var, tac::Operator::ARRAY, t3));
                
                return {t1, t2};
            }
            default:
                ASSERT_PATH_NOT_TAKEN("void is not a variable");
        }
    }

    result_type operator()(ast::Expression& value) const {
        Type type = ast::GetTypeVisitor()(value);

        if(type == BaseType::INT){
            return {performIntOperation(value, function)};
        } else if(type == BaseType::FLOAT){
            return {performFloatOperation(value, function)};
        } else if(type == BaseType::BOOL){
            return {performBoolOperation(value, function)};
        } else {
            auto t1 = function->context->newTemporary();
            auto t2 = function->context->newTemporary();

            performStringOperation(value, function, t1, t2);
            
            return {t1, t2};
        }
    }

    result_type operator()(ast::Minus& value) const {
        tac::Argument arg = moveToArgument(value.Content->value, function);
        
        Type type = visit(ast::GetTypeVisitor(), value.Content->value);

        if(type == BaseType::FLOAT){
            auto t1 = function->context->newFloatTemporary();
            function->add(std::make_shared<tac::Quadruple>(t1, arg, tac::Operator::FMINUS));

            return {t1};
        } else {
            auto t1 = function->context->newTemporary();
            function->add(std::make_shared<tac::Quadruple>(t1, arg, tac::Operator::MINUS));

            return {t1};
        }
    }
    
    result_type operator()(ast::Cast& cast) const {
        tac::Argument arg = moveToArgument(cast.Content->value, function);
        
        Type srcType = visit(ast::GetTypeVisitor(), cast.Content->value);
        Type destType = visit(ast::TypeTransformer(), cast.Content->type);

        if(srcType != destType){
            if(destType == BaseType::FLOAT){
                auto t1 = function->context->newFloatTemporary();

                function->add(std::make_shared<tac::Quadruple>(t1, arg, tac::Operator::I2F));

                return {t1};
            } else if(destType == BaseType::INT){
                auto t1 = function->context->newTemporary();
                
                function->add(std::make_shared<tac::Quadruple>(t1, arg, tac::Operator::F2I));

                return {t1};
            }
        }

        //If srcType == destType, there is nothing to do
        return {arg};
    }

    //No operation to do
    result_type operator()(ast::Plus& value) const {
        return visit(*this, value.Content->value);
    }
};

struct AbstractVisitor : public boost::static_visitor<> {
    AbstractVisitor(std::shared_ptr<tac::Function> f) : function(f) {}
    
    mutable std::shared_ptr<tac::Function> function;
    
    virtual void intAssign(std::vector<tac::Argument> arguments) const = 0;
    virtual void floatAssign(std::vector<tac::Argument> arguments) const = 0;
    virtual void stringAssign(std::vector<tac::Argument> arguments) const = 0;
   
    /* Litterals are always strings */
    
    void operator()(ast::Litteral& litteral) const {
        stringAssign(ToArgumentsVisitor(function)(litteral));
    }

    /* Can be of two types */
    
    template<typename T>
    void complexAssign(Type type, T& value) const {
        switch(type.base()){
            case BaseType::INT:
                intAssign(ToArgumentsVisitor(function)(value));
                break;
            case BaseType::BOOL:
                intAssign(ToArgumentsVisitor(function)(value));
                break;
            case BaseType::STRING:
                stringAssign(ToArgumentsVisitor(function)(value));
                break;
            case BaseType::FLOAT:
                floatAssign(ToArgumentsVisitor(function)(value));
                break;
            default:
                throw SemanticalException("Invalid variable type");   
        }
    }

    void operator()(ast::FunctionCall& call) const {
        auto type = call.Content->function->returnType;

        complexAssign(type, call);
    }

    void operator()(ast::VariableValue& value) const {
        auto type = value.Content->var->type();

        complexAssign(type, value);
    }

    void operator()(ast::ArrayValue& array) const {
        auto type = array.Content->var->type();

        complexAssign(type, array);
    }

    template<typename T>
    void operator()(T& value) const {
        auto type = ast::GetTypeVisitor()(value);
        
        complexAssign(type, value);
    }
};

struct AssignValueToArray : public AbstractVisitor {
    AssignValueToArray(std::shared_ptr<tac::Function> f, std::shared_ptr<Variable> v, ast::Value& i) : AbstractVisitor(f), variable(v), indexValue(i) {}
    
    std::shared_ptr<Variable> variable;
    ast::Value& indexValue;

    void intAssign(std::vector<tac::Argument> arguments) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 

        function->add(std::make_shared<tac::Quadruple>(variable, index, tac::Operator::ARRAY_ASSIGN, arguments[0]));
    }
    
    void floatAssign(std::vector<tac::Argument> arguments) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 

        function->add(std::make_shared<tac::Quadruple>(variable, index, tac::Operator::ARRAY_ASSIGN, arguments[0]));
    }

    void stringAssign(std::vector<tac::Argument> arguments) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 
        
        function->add(std::make_shared<tac::Quadruple>(variable, index, tac::Operator::ARRAY_ASSIGN, arguments[0]));

        auto temp1 = function->context->newTemporary();
        function->add(std::make_shared<tac::Quadruple>(temp1, index, tac::Operator::ADD, -size(BaseType::INT)));
        function->add(std::make_shared<tac::Quadruple>(variable, temp1, tac::Operator::ARRAY_ASSIGN, arguments[1]));
    }
};
 
struct AssignValueToVariable : public AbstractVisitor {
    AssignValueToVariable(std::shared_ptr<tac::Function> f, std::shared_ptr<Variable> v) : AbstractVisitor(f), variable(v) {}
    
    std::shared_ptr<Variable> variable;

    void intAssign(std::vector<tac::Argument> arguments) const {
        function->add(std::make_shared<tac::Quadruple>(variable, arguments[0], tac::Operator::ASSIGN));
    }
    
    void floatAssign(std::vector<tac::Argument> arguments) const {
        function->add(std::make_shared<tac::Quadruple>(variable, arguments[0], tac::Operator::FASSIGN));
    }

    void stringAssign(std::vector<tac::Argument> arguments) const {
        function->add(std::make_shared<tac::Quadruple>(variable, arguments[0], tac::Operator::ASSIGN));
        function->add(std::make_shared<tac::Quadruple>(variable, getStringOffset(variable), tac::Operator::DOT_ASSIGN, arguments[1]));
    }
};

struct AssignValueToVariableWithOffset : public AbstractVisitor {
    AssignValueToVariableWithOffset(std::shared_ptr<tac::Function> f, std::shared_ptr<Variable> v, unsigned int offset) : AbstractVisitor(f), variable(v), offset(offset) {}
    
    std::shared_ptr<Variable> variable;
    unsigned int offset;

    void intAssign(std::vector<tac::Argument> arguments) const {
        function->add(std::make_shared<tac::Quadruple>(variable, offset, tac::Operator::DOT_ASSIGN, arguments[0]));
    }
    
    void floatAssign(std::vector<tac::Argument> arguments) const {
        //TODO Handle floats
        function->add(std::make_shared<tac::Quadruple>(variable, arguments[0], tac::Operator::FASSIGN));
    }

    void stringAssign(std::vector<tac::Argument> arguments) const {
        function->add(std::make_shared<tac::Quadruple>(variable, offset, tac::Operator::DOT_ASSIGN, arguments[0]));
        function->add(std::make_shared<tac::Quadruple>(variable, offset + getStringOffset(variable), tac::Operator::DOT_ASSIGN, arguments[1]));
    }
};

void assign(std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> variable, ast::Value& value){
    visit(AssignValueToVariable(function, variable), value);
}

struct JumpIfFalseVisitor : public boost::static_visitor<> {
    JumpIfFalseVisitor(std::shared_ptr<tac::Function> f, const std::string& l) : function(f), label(l) {}
    
    mutable std::shared_ptr<tac::Function> function;
    std::string label;
   
    void operator()(ast::Expression& value) const ;
    
    template<typename T>
    void operator()(T& value) const {
        auto argument = ToArgumentsVisitor(function)(value)[0];

        function->add(std::make_shared<tac::IfFalse>(argument, label));
    }
};

template<typename Control>
void compare(ast::Expression& value, ast::Operator op, std::shared_ptr<tac::Function> function, const std::string& label){
    ASSERT(value.Content->operations.size() == 1, "Relational operations cannot be chained");

    auto left = moveToArgument(value.Content->first, function);
    auto right = moveToArgument(value.Content->operations[0].get<1>(), function);

    Type typeLeft = visit(ast::GetTypeVisitor(), value.Content->first);
    Type typeRight = visit(ast::GetTypeVisitor(), value.Content->operations[0].get<1>());

    ASSERT(typeLeft == typeRight, "Only values of the same type can be compared");
    ASSERT(typeLeft == BaseType::INT || typeLeft == BaseType::FLOAT, "Only int and floats can be compared");

    if(typeLeft == BaseType::INT){
        function->add(std::make_shared<Control>(tac::toBinaryOperator(op), left, right, label));
    } else if(typeLeft == BaseType::FLOAT){
        function->add(std::make_shared<Control>(tac::toFloatBinaryOperator(op), left, right, label));
    } 
}

struct JumpIfTrueVisitor : public boost::static_visitor<> {
    JumpIfTrueVisitor(std::shared_ptr<tac::Function> f, const std::string& l) : function(f), label(l) {}
    
    mutable std::shared_ptr<tac::Function> function;
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
            compare<tac::If>(value, op, function, label);
        } 
        //A bool value
        else { //Perform int operations
            auto var = performIntOperation(value, function);
            
            function->add(std::make_shared<tac::If>(var, label));
        }
    }
   
    template<typename T>
    void operator()(T& value) const {
        auto argument = ToArgumentsVisitor(function)(value)[0];

        function->add(std::make_shared<tac::If>(argument, label));
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
        compare<tac::IfFalse>(value, op, function, label);
    } 
    //A bool value
    else { //Perform int operations
        auto var = performIntOperation(value, function);

        function->add(std::make_shared<tac::IfFalse>(var, label));
    }
}

void performStringOperation(ast::Expression& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2){
    ASSERT(value.Content->operations.size() > 0, "Expression with no operation should have been transformed");

    std::vector<tac::Argument> arguments;

    auto first = visit(ToArgumentsVisitor(function), value.Content->first);
    arguments.insert(arguments.end(), first.begin(), first.end());

    //Perfom all the additions
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        auto second = visit(ToArgumentsVisitor(function), operation.get<1>());
        arguments.insert(arguments.end(), second.begin(), second.end());
        
        for(auto& arg : arguments){
            function->add(std::make_shared<tac::Param>(arg));   
        }

        arguments.clear();

        if(i == value.Content->operations.size() - 1){
            function->add(std::make_shared<tac::Call>("concat", symbols->getFunction("_F6concatSS"), v1, v2)); 
        } else {
            auto t1 = value.Content->context->newTemporary();
            auto t2 = value.Content->context->newTemporary();
            
            function->add(std::make_shared<tac::Call>("concat", symbols->getFunction("_F6concatSS"), t1, t2)); 
          
            arguments.push_back(t1);
            arguments.push_back(t2);
        }
    }
}

class CompilerVisitor : public boost::static_visitor<> {
    private:
        StringPool& pool;
        tac::Program& program;

        std::shared_ptr<tac::Function> function;
    
    public:
        CompilerVisitor(StringPool& p, tac::Program& tacProgram) : pool(p), program(tacProgram){}
        
        void operator()(ast::SourceFile& p){
            program.context = p.Content->context;

            visit_each(*this, p.Content->blocks);
        }

        void operator()(ast::FunctionDeclaration& f){
            function = std::make_shared<tac::Function>(f.Content->context, f.Content->mangledName);
            function->definition = symbols->getFunction(f.Content->mangledName);

            visit_each(*this, f.Content->instructions);

            program.functions.push_back(function);
        }

        void operator()(ast::GlobalVariableDeclaration&){
            //Nothing to compile, the global variable values are written using global contexts
        }
        
        void operator()(ast::GlobalArrayDeclaration&){
            //Nothing to compile, the global arrays are written using global contexts
        }

        void operator()(ast::ArrayDeclaration&){
            //Nothing to compile there, everything is done by the function context
        }

        void operator()(ast::Struct&){
            //Nothing to compile there
        }

        void operator()(ast::If& if_){
            if (if_.Content->elseIfs.empty()) {
                std::string endLabel = newLabel();

                visit(JumpIfFalseVisitor(function, endLabel), if_.Content->condition);

                visit_each(*this, if_.Content->instructions);

                if (if_.Content->else_) {
                    std::string elseLabel = newLabel();

                    function->add(std::make_shared<tac::Goto>(elseLabel));

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

                function->add(std::make_shared<tac::Goto>(end));

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

                    function->add(std::make_shared<tac::Goto>(end));
                }

                if (if_.Content->else_) {
                    function->add(next);

                    visit_each(*this, (*if_.Content->else_).instructions);
                }

                function->add(end);
            }
        }

        void operator()(ast::Assignment& assignment){
            assign(function, assignment.Content->context->getVariable(assignment.Content->variableName), assignment.Content->value);
        }

        void operator()(ast::CompoundAssignment&){
            //There should be no more compound assignment there as they are transformed before into Assignement with composed value
            ASSERT_PATH_NOT_TAKEN("Compound assignment should be transformed into Assignment");
        }
        
        void operator()(ast::StructCompoundAssignment&){
            //There should be no more compound assignment there as they are transformed before into Assignement with composed value
            ASSERT_PATH_NOT_TAKEN("Struct compound assignment should be transformed into Assignment");
        }
        
        void operator()(ast::ArrayAssignment& assignment){
            visit(AssignValueToArray(function, assignment.Content->context->getVariable(assignment.Content->variableName), assignment.Content->indexValue), assignment.Content->value);
        }

        void operator()(ast::StructAssignment& assignment){
            auto struct_name = (*assignment.Content->context)[assignment.Content->variableName]->type().type();
            auto struct_type = symbols->get_struct(struct_name);
            auto offset = symbols->member_offset(struct_type, assignment.Content->memberName);

            visit(AssignValueToVariableWithOffset(function, assignment.Content->context->getVariable(assignment.Content->variableName), offset), assignment.Content->value);
        }

        void operator()(ast::VariableDeclaration& declaration){
            if(declaration.Content->value){
                if(!declaration.Content->context->getVariable(declaration.Content->variableName)->type().isConst()){
                    visit(AssignValueToVariable(function, declaration.Content->context->getVariable(declaration.Content->variableName)), *declaration.Content->value);
                }
            }
        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;
            
            auto t1 = swap.Content->context->newTemporary();

            if(lhs_var->type() == BaseType::INT || lhs_var->type() == BaseType::BOOL || lhs_var->type() == BaseType::STRING){
                function->add(std::make_shared<tac::Quadruple>(t1, rhs_var, tac::Operator::ASSIGN));  
                function->add(std::make_shared<tac::Quadruple>(rhs_var, lhs_var, tac::Operator::ASSIGN));  
                function->add(std::make_shared<tac::Quadruple>(lhs_var, t1, tac::Operator::ASSIGN));  
                
                if( lhs_var->type() == BaseType::STRING){
                    auto t2 = swap.Content->context->newTemporary();

                    //t1 = 4(b)
                    function->add(std::make_shared<tac::Quadruple>(t1, rhs_var, tac::Operator::DOT, getStringOffset(rhs_var)));  
                    //t2 = 4(a)
                    function->add(std::make_shared<tac::Quadruple>(t2, lhs_var, tac::Operator::DOT, getStringOffset(lhs_var)));  
                    //4(b) = t2
                    function->add(std::make_shared<tac::Quadruple>(rhs_var, getStringOffset(rhs_var), tac::Operator::DOT_ASSIGN, t2));  
                    //4(a) = t1
                    function->add(std::make_shared<tac::Quadruple>(lhs_var, getStringOffset(lhs_var), tac::Operator::DOT_ASSIGN, t1));  
                }
            } else {
                throw SemanticalException("Variable of invalid type");
            }
        }

        void operator()(ast::SuffixOperation& operation){
            //As we don't need the return value, we can make it prefix
            performPrefixOperation(operation, function);
        }
        
        void operator()(ast::PrefixOperation& operation){
            performPrefixOperation(operation, function);
        }

        void operator()(ast::While& while_){
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            function->add(startLabel);

            visit(JumpIfFalseVisitor(function, endLabel), while_.Content->condition);

            visit_each(*this, while_.Content->instructions);

            function->add(std::make_shared<tac::Goto>(startLabel));

            function->add(endLabel);
        }

        void operator()(ast::DoWhile& while_){
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            function->add(startLabel);

            visit_each(*this, while_.Content->instructions);

            visit(JumpIfTrueVisitor(function, startLabel), while_.Content->condition);
        }

        void operator()(ast::For for_){
            visit_optional(*this, for_.Content->start);

            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            function->add(startLabel);

            if(for_.Content->condition){
                visit(JumpIfFalseVisitor(function, endLabel), *for_.Content->condition);
            }

            visit_each(*this, for_.Content->instructions);

            visit_optional(*this, for_.Content->repeat);

            function->add(std::make_shared<tac::Goto>(startLabel));
            
            function->add(endLabel);
        }

        void operator()(ast::Foreach&){
            //This node has been transformed into a for node
            ASSERT_PATH_NOT_TAKEN("Foreach should have been transformed into a For loop"); 
        }
       
        void operator()(ast::ForeachIn& foreach){
            auto iterVar = foreach.Content->iterVar;
            auto arrayVar = foreach.Content->arrayVar;
            auto var = foreach.Content->var;

            auto startLabel = newLabel();
            auto endLabel = newLabel();

            auto stringTemp = foreach.Content->context->newTemporary();

            //Init the index to 0
            function->add(std::make_shared<tac::Quadruple>(iterVar, 0, tac::Operator::ASSIGN));

            function->add(startLabel);

            auto sizeTemp = computeLengthOfArray(arrayVar, function);

            function->add(std::make_shared<tac::IfFalse>(tac::BinaryOperator::LESS, iterVar, sizeTemp, endLabel));
            
            auto indexTemp = computeIndexOfArray(arrayVar, iterVar, function);

            if(var->type() == BaseType::INT || var->type() == BaseType::BOOL){
                function->add(std::make_shared<tac::Quadruple>(var, arrayVar, tac::Operator::ARRAY, indexTemp));
            } else {
                function->add(std::make_shared<tac::Quadruple>(var, arrayVar, tac::Operator::ARRAY, indexTemp));

                auto t1 = function->context->newTemporary();

                //Assign the second part of the string
                function->add(std::make_shared<tac::Quadruple>(t1, indexTemp, tac::Operator::ADD, -size(BaseType::INT)));
                function->add(std::make_shared<tac::Quadruple>(stringTemp, arrayVar, tac::Operator::ARRAY, t1));
                function->add(std::make_shared<tac::Quadruple>(var, getStringOffset(var), tac::Operator::DOT_ASSIGN, stringTemp));
            }

            visit_each(*this, foreach.Content->instructions);    

            function->add(std::make_shared<tac::Quadruple>(iterVar, iterVar, tac::Operator::ADD, 1)); 

            function->add(std::make_shared<tac::Goto>(startLabel));
           
            function->add(endLabel); 
        }

        void operator()(ast::FunctionCall& functionCall){
            executeCall(functionCall, function, {}, {});
        }

        void operator()(ast::StandardImport&){
            //Nothing to do with imports
        }

        void operator()(ast::Import&){
            //Nothing to do with imports
        }

        void operator()(ast::Return& return_){
            auto arguments = visit(ToArgumentsVisitor(function), return_.Content->value);

            if(arguments.size() == 1){
                function->add(std::make_shared<tac::Quadruple>(tac::Operator::RETURN, arguments[0]));
            } else if(arguments.size() == 2){
                function->add(std::make_shared<tac::Quadruple>(tac::Operator::RETURN, arguments[0], arguments[1]));
            } else {
                throw SemanticalException("Invalid number of arguments");
            }   
        }
};

tac::Argument moveToArgument(ast::Value& value, std::shared_ptr<tac::Function> function){
    return visit(ToArgumentsVisitor(function), value)[0];
}

void executeCall(ast::FunctionCall& functionCall, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_){
    std::vector<std::vector<tac::Argument>> arguments;

    for(auto& value : functionCall.Content->values){
        arguments.push_back(visit(ToArgumentsVisitor(function), value)); 
    }
    
    auto functionName = mangle(functionCall.Content->functionName, functionCall.Content->values);
    auto definition = symbols->getFunction(functionName);

    ASSERT(definition, "All the functions should be in the function table");

    auto context = definition->context;

    //If it's a standard function, there are no context
    if(!context){
        auto parameters = definition->parameters;
        int i = 0;

        for(auto& first : arguments){
            auto param = parameters[i++].name; 

            for(auto& arg : first){
                function->add(std::make_shared<tac::Param>(arg, param, definition));   
            }
        }
    } else {
        auto parameters = definition->parameters;
        int i = 0;

        for(auto& first : arguments){
            std::shared_ptr<Variable> param = context->getVariable(parameters[i++].name);

            for(auto& arg : first){
                function->add(std::make_shared<tac::Param>(arg, param, definition));   
            }
        }
    }

    function->add(std::make_shared<tac::Call>(functionName, definition, return_, return2_));
}

std::shared_ptr<Variable> performBoolOperation(ast::Expression& value, std::shared_ptr<tac::Function> function){
    auto t1 = function->context->newTemporary(); 
   
    //The first operator defines the kind of operation 
    auto op = value.Content->operations[0].get<0>();

    ASSERT(op == ast::Operator::AND || op == ast::Operator::OR || (op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS), "Invalid operator on bool");

    //Logical and operators (&&)
    if(op == ast::Operator::AND){
        auto falseLabel = newLabel();
        auto endLabel = newLabel();

        visit(JumpIfFalseVisitor(function, falseLabel), value.Content->first);

        for(auto& operation : value.Content->operations){
            visit(JumpIfFalseVisitor(function, falseLabel), operation.get<1>());
        }

        function->add(std::make_shared<tac::Quadruple>(t1, 1, tac::Operator::ASSIGN));
        function->add(std::make_shared<tac::Goto>(endLabel));

        function->add(falseLabel);
        function->add(std::make_shared<tac::Quadruple>(t1, 0, tac::Operator::ASSIGN));

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

        function->add(std::make_shared<tac::Quadruple>(t1, 0, tac::Operator::ASSIGN));
        function->add(std::make_shared<tac::Goto>(endLabel));

        function->add(trueLabel);
        function->add(std::make_shared<tac::Quadruple>(t1, 1, tac::Operator::ASSIGN));

        function->add(endLabel);
    }
    //Relational operators 
    else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
        ASSERT(value.Content->operations.size() == 1, "Relational operations cannot be chained");

        auto left = moveToArgument(value.Content->first, function);
        auto right = moveToArgument(value.Content->operations[0].get<1>(), function);
        
        Type typeLeft = visit(ast::GetTypeVisitor(), value.Content->first);
        Type typeRight = visit(ast::GetTypeVisitor(), value.Content->operations[0].get<1>());

        ASSERT(typeLeft == typeRight, "Only values of the same type can be compared");
        ASSERT(typeLeft == BaseType::INT || typeLeft == BaseType::FLOAT, "Only float and int values can be compared");

        if(typeLeft == BaseType::INT){
            function->add(std::make_shared<tac::Quadruple>(t1, left, tac::toRelationalOperator(op), right));
        } else if(typeRight == BaseType::FLOAT){
            function->add(std::make_shared<tac::Quadruple>(t1, left, tac::toFloatRelationalOperator(op), right));
        }
    } 
    
    return t1;
}


} //end of anonymous namespace

void tac::Compiler::compile(ast::SourceFile& program, StringPool& pool, tac::Program& tacProgram, SymbolTable& table) const {
    symbols = &table;

    CompilerVisitor visitor(pool, tacProgram);
    visitor(program);
}
