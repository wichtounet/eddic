//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "VisitorUtils.hpp"
#include "Variable.hpp"
#include "SemanticalException.hpp"
#include "FunctionContext.hpp"
#include "GetTypeVisitor.hpp"
#include "GetConstantValue.hpp"
#include "mangling.hpp"
#include "Labels.hpp"

#include "tac/Compiler.hpp"
#include "tac/Program.hpp"

#include "ast/SourceFile.hpp"

using namespace eddic;

namespace {

struct IsSingleArgumentVisitor : public boost::static_visitor<bool> {
    ASSIGN(ast::VariableValue, true)
    ASSIGN(ast::Integer, true)
    ASSIGN(ast::True, true)
    ASSIGN(ast::False, true)
    
    ASSIGN(ast::Litteral, false)
    ASSIGN(ast::ArrayValue, false)
    ASSIGN(ast::ComposedValue, false)
    ASSIGN(ast::Minus, false)
    ASSIGN(ast::Plus, false)

    //A call to a function returning an int is single argument
    bool operator()(ast::FunctionCall& call) const {
        Type type = call.Content->function->returnType;

        return type.base() == BaseType::INT;
    }
};

//TODO In some cases, it's possible that some of them can be param safe
//Typically when their subcomponents are safe or constants
struct IsParamSafeVisitor : public boost::static_visitor<bool> {
    ASSIGN(ast::VariableValue, true)
    ASSIGN(ast::Integer, true)
    ASSIGN(ast::True, true)
    ASSIGN(ast::False, true)
    ASSIGN(ast::Litteral, true)
    
    ASSIGN(ast::ArrayValue, false)
    ASSIGN(ast::ComposedValue, false)
    ASSIGN(ast::Minus, false)
    ASSIGN(ast::Plus, false)
    ASSIGN(ast::FunctionCall, false)
};

void performStringOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2);
void executeCall(ast::FunctionCall& functionCall, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);
tac::Argument moveToArgument(ast::Value& value, std::shared_ptr<tac::Function> function);

std::shared_ptr<Variable> performIntOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function){
    assert(value.Content->operations.size() > 0); //This has been enforced by previous phases

    tac::Argument left = moveToArgument(value.Content->first, function);
    tac::Argument right;

    auto t1 = function->context->newTemporary(); 

    //Apply all the operations in chain
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        right = moveToArgument(operation.get<1>(), function);
       
        if (i == 0){
            function->add(std::make_shared<tac::Quadruple>(t1, left, tac::toOperator(operation.get<0>()), right));
        } else {
            function->add(std::make_shared<tac::Quadruple>(t1, t1, tac::toOperator(operation.get<0>()), right));
        }
    }

    return t1;
}

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
        function->add(std::make_shared<tac::Quadruple>(t1, array->type().size()));
    } else if(position.isParameter()){
        function->add(std::make_shared<tac::Quadruple>(t1, array, tac::Operator::ARRAY, 0));
    }

    return t1;
}

int getStringOffset(std::shared_ptr<Variable> variable){
    return variable->position().isGlobal() ? 4 : -4;
}

struct ToArgumentsVisitor : public boost::static_visitor<std::vector<tac::Argument>> {
    ToArgumentsVisitor(std::shared_ptr<tac::Function> f) : function(f) {}
    
    mutable std::shared_ptr<tac::Function> function;

    result_type operator()(ast::Litteral& litteral) const {
        return {litteral.label, litteral.value.size() - 2};
    }

    result_type operator()(ast::Integer& integer) const {
        return {integer.value};
    }
    
    result_type operator()(ast::False&) const {
        return {0};
    }
    
    result_type operator()(ast::True&) const {
        return {1};
    }

    result_type operator()(ast::FunctionCall& call) const {
        Type type = call.Content->function->returnType;

        switch(type.base()){
            case BaseType::INT:{
                auto t1 = function->context->newTemporary();

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

    result_type operator()(ast::VariableValue& value) const {
        auto type = value.Content->var->type();

        //If it's a const, we just have to replace it by its constant value
        if(type.isConst()){
           auto val = value.Content->var->val();
           
           if(type.base() == BaseType::INT){
               return {boost::get<int>(val)};
           } else if (type.base() == BaseType::STRING){
                auto value = boost::get<std::pair<std::string, int>>(val);

                return {value.first, value.second};
           }
        } else if(type.isArray()){
            return {value.Content->var};
        } else {
            if(type.base() == BaseType::INT){
                return {value.Content->var};
            } else {
                auto temp = value.Content->context->newTemporary();
                function->add(std::make_shared<tac::Quadruple>(temp, value.Content->var, tac::Operator::DOT, getStringOffset(value.Content->var)));

                return {value.Content->var, temp};
            }
        }

        assert(false);
    }

    result_type operator()(ast::ArrayValue& array) const {
        auto index = computeIndexOfArray(array.Content->var, array.Content->indexValue, function); 

        if(array.Content->var->type().base() == BaseType::INT){
            auto temp = array.Content->context->newTemporary();
            function->add(std::make_shared<tac::Quadruple>(temp, array.Content->var, tac::Operator::ARRAY, index));
            
            return {temp};
        } else {
            auto t1 = array.Content->context->newTemporary();
            function->add(std::make_shared<tac::Quadruple>(t1, array.Content->var, tac::Operator::ARRAY, index));
                
            auto t2 = array.Content->context->newTemporary();
            auto t3 = array.Content->context->newTemporary();
            
            //Assign the second part of the string
            function->add(std::make_shared<tac::Quadruple>(t3, index, tac::Operator::ADD, -4));
            function->add(std::make_shared<tac::Quadruple>(t2, array.Content->var, tac::Operator::ARRAY, t3));
            
            return {t1, t2};
        }
    }

    result_type operator()(ast::ComposedValue& value) const {
        Type type = GetTypeVisitor()(value);

        if(type.base() == BaseType::INT){
            return {performIntOperation(value, function)};
        } else {
            auto t1 = function->context->newTemporary();
            auto t2 = function->context->newTemporary();

            performStringOperation(value, function, t1, t2);
            
            return {t1, t2};
        }
    }

    result_type operator()(ast::Minus& value) const {
        tac::Argument arg = moveToArgument(value.Content->value, function);

        auto t1 = function->context->newTemporary();
        function->add(std::make_shared<tac::Quadruple>(t1, arg, tac::Operator::MINUS));

        return {t1};
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
    virtual void stringAssign(std::vector<tac::Argument> arguments) const = 0;
   
    /* Litterals are always strings */
    
    void operator()(ast::Litteral& litteral) const {
        stringAssign(ToArgumentsVisitor(function)(litteral));
    }

    /* Can be of two types */
    
    template<typename T>
    void complexAssign(Type type, T& value) const {
        if(type.base() == BaseType::INT){
            intAssign(ToArgumentsVisitor(function)(value));
        } else if(type.base() == BaseType::STRING){
            stringAssign(ToArgumentsVisitor(function)(value));
        } else {
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

    void operator()(ast::ComposedValue& value) const {
        auto type = GetTypeVisitor()(value);
        
        complexAssign(type, value);
    }

    /* Only int */

    template<typename T>
    void operator()(T& value) const {
        intAssign(ToArgumentsVisitor(function)(value));
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

    void stringAssign(std::vector<tac::Argument> arguments) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 
        
        function->add(std::make_shared<tac::Quadruple>(variable, index, tac::Operator::ARRAY_ASSIGN, arguments[0]));

        auto temp1 = function->context->newTemporary();
        function->add(std::make_shared<tac::Quadruple>(temp1, index, tac::Operator::ADD, -4));
        function->add(std::make_shared<tac::Quadruple>(variable, temp1, tac::Operator::ARRAY_ASSIGN, arguments[1]));
    }
};
 
struct AssignValueToVariable : public AbstractVisitor {
    AssignValueToVariable(std::shared_ptr<tac::Function> f, std::shared_ptr<Variable> v) : AbstractVisitor(f), variable(v) {}
    
    std::shared_ptr<Variable> variable;

    void intAssign(std::vector<tac::Argument> arguments) const {
        function->add(std::make_shared<tac::Quadruple>(variable, arguments[0]));
    }

    void stringAssign(std::vector<tac::Argument> arguments) const {
        function->add(std::make_shared<tac::Quadruple>(variable, arguments[0]));
        function->add(std::make_shared<tac::Quadruple>(variable, getStringOffset(variable), tac::Operator::DOT_ASSIGN, arguments[1]));
    }
};

struct JumpIfTrueVisitor : public boost::static_visitor<> {
    JumpIfTrueVisitor(std::shared_ptr<tac::Function> f, const std::string& l) : function(f), label(l) {}
    
    mutable std::shared_ptr<tac::Function> function;
    std::string label;
   
    void operator()(ast::ComposedValue& value) const {
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
            //TODO
        }
        //Relational operators 
        else if(op >= ast::Operator::EQUALS && op <= ast::Operator::GREATER_EQUALS){
            //relational operations cannot be chained
            assert(value.Content->operations.size() == 1);
            
            auto left = moveToArgument(value.Content->first, function);
            auto right = moveToArgument(value.Content->operations[0].get<1>(), function);

            function->add(std::make_shared<tac::If>(tac::toBinaryOperator(op), left, right, label));
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

struct JumpIfFalseVisitor : public boost::static_visitor<> {
    JumpIfFalseVisitor(std::shared_ptr<tac::Function> f, const std::string& l) : function(f), label(l) {}
    
    mutable std::shared_ptr<tac::Function> function;
    std::string label;
   
    void operator()(ast::ComposedValue& value) const {
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
            //relational operations cannot be chained
            assert(value.Content->operations.size() == 1);
            
            auto left = moveToArgument(value.Content->first, function);
            auto right = moveToArgument(value.Content->operations[0].get<1>(), function);

            function->add(std::make_shared<tac::IfFalse>(tac::toBinaryOperator(op), left, right, label));
        } 
        //A bool value
        else { //Perform int operations
            auto var = performIntOperation(value, function);
            
            function->add(std::make_shared<tac::IfFalse>(var, label));
        }
    }
   
    template<typename T>
    void operator()(T& value) const {
        auto argument = ToArgumentsVisitor(function)(value)[0];

        function->add(std::make_shared<tac::IfFalse>(argument, label));
    }
};

void performStringOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2){
    assert(value.Content->operations.size() > 0); //Other values must be transformed before that phase

    std::vector<tac::Argument> arguments;

    auto first = visit(ToArgumentsVisitor(function), value.Content->first);
    arguments.insert(arguments.end(), first.begin(), first.end());

    //Perfom all the additions
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        auto second = visit(ToArgumentsVisitor(function), operation.get<1>());
        arguments.insert(arguments.end(), second.begin(), second.end());
        
        for(auto& arg : arguments){
            function->add(std::make_shared<tac::Quadruple>(tac::Operator::PARAM, arg));   
        }

        arguments.clear();

        if(i == value.Content->operations.size() - 1){
            function->add(std::make_shared<tac::Call>("concat", 16, v1, v2)); 
        } else {
            auto t1 = value.Content->context->newTemporary();
            auto t2 = value.Content->context->newTemporary();
            
            function->add(std::make_shared<tac::Call>("concat", 16, t1, t2)); 
          
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
        CompilerVisitor(StringPool& p, tac::Program& tacProgram) : pool(p), program(tacProgram) {}
        
        void operator()(ast::SourceFile& p){
            program.context = p.Content->context;

            visit_each(*this, p.Content->blocks);
        }

        void operator()(ast::FunctionDeclaration& f){
            function = std::make_shared<tac::Function>(f.Content->context, f.Content->mangledName);

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
            visit(AssignValueToVariable(function, assignment.Content->context->getVariable(assignment.Content->variableName)), assignment.Content->value);
        }
        
        void operator()(ast::ArrayAssignment& assignment){
            visit(AssignValueToArray(function, assignment.Content->context->getVariable(assignment.Content->variableName), assignment.Content->indexValue), assignment.Content->value);
        }

        void operator()(ast::VariableDeclaration& declaration){
            if(!declaration.Content->context->getVariable(declaration.Content->variableName)->type().isConst()){
                visit(AssignValueToVariable(function, declaration.Content->context->getVariable(declaration.Content->variableName)), *declaration.Content->value);
            }
        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;
            
            auto t1 = swap.Content->context->newTemporary();

            if(lhs_var->type().base() == BaseType::INT || lhs_var->type().base() == BaseType::STRING){
                function->add(std::make_shared<tac::Quadruple>(t1, rhs_var));  
                function->add(std::make_shared<tac::Quadruple>(rhs_var, lhs_var));  
                function->add(std::make_shared<tac::Quadruple>(lhs_var, t1));  
                
                if( lhs_var->type().base() == BaseType::STRING){
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

        void operator()(ast::While& while_){
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            function->add(startLabel);

            visit(JumpIfFalseVisitor(function, endLabel), while_.Content->condition);

            visit_each(*this, while_.Content->instructions);

            function->add(std::make_shared<tac::Goto>(startLabel));

            function->add(endLabel);
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
            assert(false); //This node has been transformed into a for node
        }
       
        void operator()(ast::ForeachIn& foreach){
            auto iterVar = foreach.Content->iterVar;
            auto arrayVar = foreach.Content->arrayVar;
            auto var = foreach.Content->var;

            auto startLabel = newLabel();
            auto endLabel = newLabel();

            auto stringTemp = foreach.Content->context->newTemporary();

            //Init the index to 0
            function->add(std::make_shared<tac::Quadruple>(iterVar, 0));

            function->add(startLabel);

            auto sizeTemp = computeLengthOfArray(arrayVar, function);

            function->add(std::make_shared<tac::IfFalse>(tac::BinaryOperator::LESS, iterVar, sizeTemp, endLabel));
            
            auto indexTemp = computeIndexOfArray(arrayVar, iterVar, function);

            if(var->type().base() == BaseType::INT){
                function->add(std::make_shared<tac::Quadruple>(var, arrayVar, tac::Operator::ARRAY, indexTemp));
            } else {
                function->add(std::make_shared<tac::Quadruple>(var, arrayVar, tac::Operator::ARRAY, indexTemp));

                auto t1 = function->context->newTemporary();

                //Assign the second part of the string
                function->add(std::make_shared<tac::Quadruple>(t1, indexTemp, tac::Operator::ADD, -4));
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
                function->add(std::make_shared<tac::Return>(arguments[0]));
            } else if(arguments.size() == 2){
                function->add(std::make_shared<tac::Return>(arguments[0], arguments[1]));
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

    for(auto& first : arguments){
        for(auto& arg : first){
            function->add(std::make_shared<tac::Quadruple>(tac::Operator::PARAM, arg));   
        }
    }

    std::string functionName;  
    if(functionCall.Content->functionName == "print" || functionCall.Content->functionName == "println"){
        Type type = visit(GetTypeVisitor(), functionCall.Content->values[0]);

        if(type.base() == BaseType::INT){
            functionName = "print_integer";
        } else if(type.base() == BaseType::STRING){
            functionName = "print_string";
        } else {
            assert(false);
        }
    } else {
        functionName = mangle(functionCall.Content->functionName, functionCall.Content->values);
    }

    int total = 0;
    for(auto& value : functionCall.Content->values){
        Type type = visit(GetTypeVisitor(), value);   

        if(type.isArray()){
            //Passing an array is just passing an adress
            total += size(BaseType::INT);
        } else {
            total += size(type);
        }
    }

    function->add(std::make_shared<tac::Call>(functionName, total, return_, return2_));
    
    if(functionCall.Content->functionName == "println"){
        function->add(std::make_shared<tac::Call>("print_line", 0));
    }
}

} //end of anonymous namespace

void tac::Compiler::compile(ast::SourceFile& program, StringPool& pool, tac::Program& tacProgram) const {
    CompilerVisitor visitor(pool, tacProgram);
    visitor(program);
}
