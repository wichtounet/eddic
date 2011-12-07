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
#include "mangling.hpp"

#include "tac/TacCompiler.hpp"
#include "tac/Program.hpp"

#include "ast/Program.hpp"

//TODO Move label generator system in another folder
#include "il/Labels.hpp"

using namespace eddic;

tac::Operator toOperator(char op){
    switch(op){
        case '+':
            return tac::Operator::ADD;
        case '-':
            return tac::Operator::SUB;
        case '/':
            return tac::Operator::DIV;
        case '*':
            return tac::Operator::MUL;
        case '%':
            return tac::Operator::MOD;
        default:
            assert(false);
    }
}

void moveToVariable(ast::Value& value, std::shared_ptr<Variable> variable, std::shared_ptr<tac::Function> function);

void performIntOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> variable){
    assert(value.Content->operations.size() > 0); //This has been enforced by previous phases

    auto t1 = value.Content->context->newTemporary();
    auto t2 = value.Content->context->newTemporary();

    moveToVariable(value.Content->first, t1, function);

    //Apply all the operations in chain
    for(auto& operation : value.Content->operations){
        moveToVariable(value.Content->first, t2, function);
        
        function->add(tac::Quadruple(t2, t1, toOperator(operation.get<0>()), t2));

        t1 = t2;
        t2 = value.Content->context->newTemporary();
    }

    function->add(tac::Quadruple(variable, t1));
}

void performStringOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2);

std::shared_ptr<Variable> computeIndexOfArray(std::shared_ptr<Variable> arrayVar, ast::Value& indexValue, std::shared_ptr<tac::Function> function){
    //TODO
}

std::shared_ptr<Variable> computeIndexOfArray(std::shared_ptr<Variable> arrayVar, std::shared_ptr<Variable> iterVar, std::shared_ptr<tac::Function> function){
    //TODO
}

std::shared_ptr<Variable> computeLengthOfArray(std::shared_ptr<Variable> arrayVar, std::shared_ptr<tac::Function> function){
    //TODO
}

void executeCall(ast::FunctionCall& functionCall, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);

struct AssignValueToArray : public boost::static_visitor<> {
    AssignValueToArray(std::shared_ptr<tac::Function> f, std::shared_ptr<Variable> v, ast::Value& i) : function(f), variable(v), indexValue(i) {}
    
    mutable std::shared_ptr<tac::Function> function;
    std::shared_ptr<Variable> variable;
    ast::Value& indexValue;

    void operator()(ast::Litteral& litteral) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 
        
        function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, litteral.label));

        auto temp1 = function->context->newTemporary();
        function->add(tac::Quadruple(temp1, index, tac::Operator::ADD, 4));
        function->add(tac::Quadruple(variable, temp1, tac::Operator::ARRAY_ASSIGN, litteral.value.size() - 2));
    }

    void operator()(ast::Integer& integer) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 

        function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, integer.value));
    }

    void operator()(ast::FunctionCall& call) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 
        Type type = call.Content->function->returnType;

        switch(type.base()){
            case BaseType::INT:{
                auto t1 = function->context->newTemporary();

                executeCall(call, function, t1, {});
        
                function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, t1));

                break;
            }
            case BaseType::STRING:{
                auto t1 = function->context->newTemporary();
                auto t2 = function->context->newTemporary();

                executeCall(call, function, t1, t2);

                function->add(tac::Quadruple(variable, 4, tac::Operator::DOT_ASSIGN, t1));
        
                auto temp1 = function->context->newTemporary();
                function->add(tac::Quadruple(temp1, index, tac::Operator::ADD, 4));
                function->add(tac::Quadruple(variable, temp1, tac::Operator::ARRAY_ASSIGN, t2));

                break;
            }
            default:
                throw SemanticalException("This function doesn't return anything");   
        }
    }

    void operator()(ast::VariableValue& value) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 
        auto type = value.Content->var->type();

        if(type.base() == BaseType::INT){
            function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, value.Content->var));
        } else if(type.base() == BaseType::STRING){
            function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, value.Content->var));

            auto temp1 = value.Content->context->newTemporary();
            auto temp2 = value.Content->context->newTemporary();
            function->add(tac::Quadruple(temp1, index, tac::Operator::ADD, 4));
            function->add(tac::Quadruple(temp2, value.Content->var, tac::Operator::DOT, 4));
            function->add(tac::Quadruple(variable, temp1, tac::Operator::ARRAY_ASSIGN, temp2));
        }
    }

    void operator()(ast::ArrayValue& array) const {
        auto index = computeIndexOfArray(variable, indexValue, function); 
        auto sourceIndex = computeIndexOfArray(array.Content->var, array.Content->indexValue, function); 

        if(array.Content->var->type().base() == BaseType::INT){
            auto t1 = function->context->newTemporary();
            function->add(tac::Quadruple(t1, array.Content->var, tac::Operator::ARRAY, sourceIndex));
            function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, t1));
        } else {
            auto t1 = function->context->newTemporary();
            function->add(tac::Quadruple(t1, array.Content->var, tac::Operator::ARRAY, sourceIndex));
            function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, t1));
                
            auto t2 = array.Content->context->newTemporary();
            
            //Assign the second part of the string
            function->add(tac::Quadruple(index, index, tac::Operator::ADD, 4));
            function->add(tac::Quadruple(t2, array.Content->var, tac::Operator::ARRAY, sourceIndex));
            function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, t2));
        }
    }

    void operator()(ast::ComposedValue& value) const {
        Type type = GetTypeVisitor()(value);

        auto index = computeIndexOfArray(variable, indexValue, function); 
        
        if(type.base() == BaseType::INT){
            auto t1 = value.Content->context->newTemporary();
            performIntOperation(value, function, t1);
            function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, t1));
        } else if(type.base() == BaseType::STRING){
            auto t1 = value.Content->context->newTemporary();
            auto t2 = value.Content->context->newTemporary();

            performStringOperation(value, function, t1, t2);
            
            function->add(tac::Quadruple(variable, index, tac::Operator::ARRAY_ASSIGN, t1));
            
            auto t3 = value.Content->context->newTemporary();
            function->add(tac::Quadruple(t3, index, tac::Operator::ADD, 4));
            function->add(tac::Quadruple(variable, t3, tac::Operator::ARRAY_ASSIGN, t2));
        }
    }
};
 
struct AssignValueToVariable : public boost::static_visitor<> {
    AssignValueToVariable(std::shared_ptr<tac::Function> f, std::shared_ptr<Variable> v) : function(f), variable(v) {}
    
    mutable std::shared_ptr<tac::Function> function;
    std::shared_ptr<Variable> variable;

    void operator()(ast::Litteral& litteral) const {
        function->add(tac::Quadruple(variable, litteral.label));
        function->add(tac::Quadruple(variable, 4, tac::Operator::DOT_ASSIGN, litteral.value.size() - 2));
    }

    void operator()(ast::Integer& integer) const {
        function->add(tac::Quadruple(variable, integer.value));
    }

    void operator()(ast::FunctionCall& call) const {
        Type type = call.Content->function->returnType;

        switch(type.base()){
            case BaseType::INT:
                executeCall(call, function, variable, {});

                break;
            case BaseType::STRING:{
                auto t1 = function->context->newTemporary();

                executeCall(call, function, variable, t1);

                function->add(tac::Quadruple(variable, 4, tac::Operator::DOT_ASSIGN, t1));

                break;
            }
            default:
                throw SemanticalException("This function doesn't return anything");   
        }
    }

    void operator()(ast::VariableValue& value) const {
        auto type = value.Content->var->type();

        if(type.base() == BaseType::INT){
            function->add(tac::Quadruple(variable, value.Content->var));
        } else if(type.base() == BaseType::STRING){
            function->add(tac::Quadruple(variable, value.Content->var));

            auto temp = value.Content->context->newTemporary();
            function->add(tac::Quadruple(temp, value.Content->var, tac::Operator::DOT, 4));
            function->add(tac::Quadruple(variable, 4, tac::Operator::DOT_ASSIGN, temp));
        }
    }

    void operator()(ast::ArrayValue& array) const {
        auto index = computeIndexOfArray(array.Content->var, array.Content->indexValue, function); 

        if(array.Content->var->type().base() == BaseType::INT){
            function->add(tac::Quadruple(variable, array.Content->var, tac::Operator::ARRAY, index));
        } else {
            function->add(tac::Quadruple(variable, array.Content->var, tac::Operator::ARRAY, index));
                
            auto t2 = array.Content->context->newTemporary();
            
            //Assign the second part of the string
            function->add(tac::Quadruple(index, index, tac::Operator::ADD, 4));
            function->add(tac::Quadruple(t2, array.Content->var, tac::Operator::ARRAY, index));
            function->add(tac::Quadruple(variable, 4, tac::Operator::DOT_ASSIGN, t2));
        }
    }

    void operator()(ast::ComposedValue& value) const {
        Type type = GetTypeVisitor()(value);

        if(type.base() == BaseType::INT){
            performIntOperation(value, function, variable);
        } else if(type.base() == BaseType::STRING){
            auto t1 = value.Content->context->newTemporary();
            performStringOperation(value, function, variable, t1);
            
            function->add(tac::Quadruple(variable, 4, tac::Operator::DOT_ASSIGN, t1));
        }
    }
};

struct PassValueAsParam : public boost::static_visitor<> {
    PassValueAsParam(std::shared_ptr<tac::Function> f) : function(f) {}
    
    mutable std::shared_ptr<tac::Function> function;

    void operator()(ast::Litteral& litteral) const {
        function->add(tac::Param(litteral.label));
        function->add(tac::Param(litteral.value.size() - 2));
    }

    void operator()(ast::Integer& integer) const {
        function->add(tac::Param(integer.value));
    }

    void operator()(ast::FunctionCall& call) const {
        Type type = call.Content->function->returnType;

        switch(type.base()){
            case BaseType::INT:{
                auto t1 = function->context->newTemporary();

                executeCall(call, function, t1, {});

                function->add(tac::Param(t1));

                break;
            }
            case BaseType::STRING:{
                auto t1 = function->context->newTemporary();
                auto t2 = function->context->newTemporary();

                executeCall(call, function, t1, t2);

                function->add(tac::Param(t1));
                function->add(tac::Param(t2));

                break;
            }
            default:
                throw SemanticalException("This function doesn't return anything");   
        }
    }

    void operator()(ast::VariableValue& value) const {
        auto type = value.Content->var->type();

        if(type.base() == BaseType::INT){
            function->add(tac::Param(value.Content->var));
        } else if(type.base() == BaseType::STRING){
            function->add(tac::Param(value.Content->var));

            auto temp = value.Content->context->newTemporary();
            function->add(tac::Quadruple(temp, value.Content->var, tac::Operator::DOT, 4));
            function->add(tac::Param(temp));
        }
    }

    void operator()(ast::ArrayValue& array) const {
        auto index = computeIndexOfArray(array.Content->var, array.Content->indexValue, function); 

        if(array.Content->var->type().base() == BaseType::INT){
            auto temp = array.Content->context->newTemporary();
            function->add(tac::Quadruple(temp, array.Content->var, tac::Operator::ARRAY, index));
            function->add(tac::Param(temp)); 
        } else {
            auto temp = array.Content->context->newTemporary();
            function->add(tac::Quadruple(temp, array.Content->var, tac::Operator::ARRAY, index));
            function->add(tac::Param(temp)); 
                
            auto t2 = array.Content->context->newTemporary();
            
            //Assign the second part of the string
            function->add(tac::Quadruple(index, index, tac::Operator::ADD, 4));
            function->add(tac::Quadruple(t2, array.Content->var, tac::Operator::ARRAY, index));
            
            function->add(tac::Param(t2)); 
        }
    }

    void operator()(ast::ComposedValue& value) const {
        Type type = GetTypeVisitor()(value);

        if(type.base() == BaseType::INT){
            auto t1 = value.Content->context->newTemporary();
            performIntOperation(value, function, t1);
            function->add(tac::Param(t1));
        } else if(type.base() == BaseType::STRING){
            auto t1 = value.Content->context->newTemporary();
            auto t2 = value.Content->context->newTemporary();

            performStringOperation(value, function, t1, t2);
            
            function->add(tac::Param(t1)); 
            function->add(tac::Param(t2)); 
        }
    }
};

struct ReturnValue : public boost::static_visitor<> {
    ReturnValue(std::shared_ptr<tac::Function> f) : function(f) {}
    
    mutable std::shared_ptr<tac::Function> function;

    void operator()(ast::Litteral& litteral) const {
        function->add(tac::Return(litteral.label, litteral.value.size() - 2));
    }

    void operator()(ast::Integer& integer) const {
        function->add(tac::Return(integer.value));
    }

    void operator()(ast::FunctionCall& call) const {
        Type type = call.Content->function->returnType;

        switch(type.base()){
            case BaseType::INT:{
                auto t1 = function->context->newTemporary();

                executeCall(call, function, t1, {});

                function->add(tac::Return(t1));

                break;
            }
            case BaseType::STRING:{
                auto t1 = function->context->newTemporary();
                auto t2 = function->context->newTemporary();

                executeCall(call, function, t1, t2);

                function->add(tac::Return(t1, t2));

                break;
            }
            default:
                throw SemanticalException("This function doesn't return anything");   
        }
    }

    void operator()(ast::VariableValue& value) const {
        auto type = value.Content->var->type();

        if(type.base() == BaseType::INT){
            function->add(tac::Return(value.Content->var));
        } else if(type.base() == BaseType::STRING){
            auto temp = value.Content->context->newTemporary();
            function->add(tac::Quadruple(temp, value.Content->var, tac::Operator::DOT, 4));

            function->add(tac::Return(value.Content->var, temp));
        }
    }

    void operator()(ast::ArrayValue& array) const {
        auto index = computeIndexOfArray(array.Content->var, array.Content->indexValue, function); 

        if(array.Content->var->type().base() == BaseType::INT){
            auto temp = array.Content->context->newTemporary();
            function->add(tac::Quadruple(temp, array.Content->var, tac::Operator::ARRAY, index));
            function->add(tac::Return(temp)); 
        } else {
            auto t1 = array.Content->context->newTemporary();
            function->add(tac::Quadruple(t1, array.Content->var, tac::Operator::ARRAY, index));
            function->add(tac::Param(t1)); 
                
            auto t2 = array.Content->context->newTemporary();
            
            //Assign the second part of the string
            function->add(tac::Quadruple(index, index, tac::Operator::ADD, 4));
            function->add(tac::Quadruple(t2, array.Content->var, tac::Operator::ARRAY, index));
            
            function->add(tac::Return(t1, t2)); 
        }
    }

    void operator()(ast::ComposedValue& value) const {
        Type type = GetTypeVisitor()(value);

        if(type.base() == BaseType::INT){
            auto t1 = value.Content->context->newTemporary();
            performIntOperation(value, function, t1);
            function->add(tac::Return(t1));
        } else if(type.base() == BaseType::STRING){
            auto t1 = value.Content->context->newTemporary();
            auto t2 = value.Content->context->newTemporary();

            performStringOperation(value, function, t1, t2);
            
            function->add(tac::Return(t1)); 
            function->add(tac::Return(t2)); 
        }
    }
};
 
struct JumpIfFalseVisitor : public boost::static_visitor<> {
    JumpIfFalseVisitor(std::shared_ptr<tac::Function> f, const std::string& l) : function(f), label(l) {}
    
    mutable std::shared_ptr<tac::Function> function;
    std::string label;
   
    void operator()(ast::True&) const {
        //it is a no-op
    }
   
    void operator()(ast::False&) const {
        //we always jump
        function->add(tac::Goto(label));
    }
   
    void operator()(ast::BinaryCondition& binaryCondition) const {
        auto t1 = function->context->newTemporary();
        auto t2 = function->context->newTemporary();

        boost::apply_visitor(AssignValueToVariable(function, t1), binaryCondition.Content->lhs);
        boost::apply_visitor(AssignValueToVariable(function, t2), binaryCondition.Content->rhs);

        function->add(tac::IfFalse(tac::toBinaryOperator(binaryCondition.Content->op), t1, t2, label));
    }
};

void moveToVariable(ast::Value& value, std::shared_ptr<Variable> variable, std::shared_ptr<tac::Function> function){
    AssignValueToVariable visitor(function, variable);
    boost::apply_visitor(visitor, value);
}

void performStringOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2){
    assert(value.Content->operations.size() > 0); //Other values must be transformed before that phase

    unsigned int iter = 0;

    PassValueAsParam pusher(function);
    visit_non_variant(pusher, value);

    //Perfom all the additions
    for(auto& operation : value.Content->operations){
        visit(pusher, operation.get<1>());

        auto t1 = value.Content->context->newTemporary();
        auto t2 = value.Content->context->newTemporary();

        //TODO result in t1, t2
        function->add(tac::Call("concat", 16)); 

        ++iter;

        //If there is more operation, push the answer
        if(iter < value.Content->operations.size()){
           function->add(tac::Param(t1));
           function->add(tac::Param(t2));
        } else {
            function->add(tac::Quadruple(v1, t1));
            function->add(tac::Quadruple(v2, t2));
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
        
        void operator()(ast::Program& p){
            program.context = p.Content->context;

            visit_each(*this, p.Content->blocks);
        }

        void operator()(ast::FunctionDeclaration& f){
            function = std::make_shared<tac::Function>(f.Content->context);

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

                boost::apply_visitor(JumpIfFalseVisitor(function, endLabel), if_.Content->condition);

                visit_each(*this, if_.Content->instructions);

                if (if_.Content->else_) {
                    std::string elseLabel = newLabel();

                    function->add(tac::Goto(elseLabel));

                    function->add(endLabel);

                    visit_each(*this, (*if_.Content->else_).instructions);

                    function->add(elseLabel);
                } else {
                    function->add(endLabel);
                }
            } else {
                std::string end = newLabel();
                std::string next = newLabel();

                boost::apply_visitor(JumpIfFalseVisitor(function, next), if_.Content->condition);

                visit_each(*this, if_.Content->instructions);

                function->add(tac::Goto(end));

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

                    boost::apply_visitor(JumpIfFalseVisitor(function, next), elseIf.condition);

                    visit_each(*this, elseIf.instructions);

                    function->add(tac::Goto(end));
                }

                if (if_.Content->else_) {
                    function->add(next);

                    visit_each(*this, (*if_.Content->else_).instructions);
                }

                function->add(end);
            }
        }

        void operator()(ast::Assignment& assignment){
            boost::apply_visitor(AssignValueToVariable(function, assignment.Content->context->getVariable(assignment.Content->variableName)), assignment.Content->value);
        }
        
        void operator()(ast::ArrayAssignment& assignment){
            boost::apply_visitor(AssignValueToArray(function, assignment.Content->context->getVariable(assignment.Content->variableName), assignment.Content->indexValue), assignment.Content->value);
        }

        void operator()(ast::VariableDeclaration& declaration){
            boost::apply_visitor(AssignValueToVariable(function, declaration.Content->context->getVariable(declaration.Content->variableName)), *declaration.Content->value);
        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;

            //We have the guarantee here that both variables are of the same type
            switch (lhs_var->type().base()) {
                case BaseType::INT:{
                    auto temp = swap.Content->context->newTemporary();

                    function->add(tac::Quadruple(temp, rhs_var));  
                    function->add(tac::Quadruple(rhs_var, lhs_var));  
                    function->add(tac::Quadruple(lhs_var, temp));  

                    break;
                }
                case BaseType::STRING:{
                    auto temp = swap.Content->context->newTemporary();

                    function->add(tac::Quadruple(temp, rhs_var));  
                    function->add(tac::Quadruple(rhs_var, lhs_var));  
                    function->add(tac::Quadruple(lhs_var, temp));  

                    function->add(tac::Quadruple(temp, rhs_var, tac::Operator::DOT, 4));  
                    function->add(tac::Quadruple(rhs_var, lhs_var, tac::Operator::DOT, 4));  
                    function->add(tac::Quadruple(lhs_var, temp));  

                    break;
                }
                default:
                   throw SemanticalException("Variable of invalid type");
            }
        }

        void operator()(ast::While& while_){
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            function->add(startLabel);

            boost::apply_visitor(JumpIfFalseVisitor(function, endLabel), while_.Content->condition);

            visit_each(*this, while_.Content->instructions);

            function->add(tac::Goto(startLabel));

            function->add(endLabel);
        }

        void operator()(ast::For for_){
            visit_optional(*this, for_.Content->start);

            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            function->add(startLabel);

            if(for_.Content->condition){
                boost::apply_visitor(JumpIfFalseVisitor(function, endLabel), *for_.Content->condition);
            }

            visit_each(*this, for_.Content->instructions);

            visit_optional(*this, for_.Content->repeat);

            function->add(tac::Goto(startLabel));
            
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
            function->add(tac::Quadruple(iterVar, 0));

            function->add(startLabel);

            auto indexTemp = computeIndexOfArray(arrayVar, iterVar, function);
            auto sizeTemp = computeLengthOfArray(arrayVar, function);

            function->add(tac::IfFalse(tac::BinaryOperator::LESS, iterVar, sizeTemp, endLabel));

            if(var->type().base() == BaseType::INT){
                function->add(tac::Quadruple(var, arrayVar, tac::Operator::ARRAY, indexTemp));
            } else {
                function->add(tac::Quadruple(var, arrayVar, tac::Operator::ARRAY, indexTemp));

                //Assign the second part of the string
                function->add(tac::Quadruple(indexTemp, indexTemp, tac::Operator::ADD, 4));
                function->add(tac::Quadruple(stringTemp, arrayVar, tac::Operator::ARRAY, indexTemp));
                function->add(tac::Quadruple(var, 4, tac::Operator::DOT_ASSIGN, stringTemp));
            }

            visit_each(*this, foreach.Content->instructions);    

            function->add(tac::Quadruple(iterVar, iterVar, tac::Operator::ADD, 1)); 

            function->add(tac::Goto(startLabel));
           
            function->add(endLabel); 
        }

        void operator()(ast::FunctionCall& functionCall){
            executeCall(functionCall, function, {}, {});
        }

        void operator()(ast::Return& return_){
            ReturnValue visitor(function);
            boost::apply_visitor(visitor, return_.Content->value);
        }
};

void executeCall(ast::FunctionCall& functionCall, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_){
    PassValueAsParam visitor(function);
    for(auto& value : functionCall.Content->values){
        boost::apply_visitor(visitor, value);
    }

    if(functionCall.Content->functionName == "print" || functionCall.Content->functionName == "println"){
        Type type = boost::apply_visitor(GetTypeVisitor(), functionCall.Content->values[0]);

        switch (type.base()) {
            case BaseType::INT:
                function->add(tac::Call("print_integer", 4));

                break;
            case BaseType::STRING:
                function->add(tac::Call("print_string", 8));

                break;
            default:
                throw SemanticalException("Variable of invalid type");
        }

        if(functionCall.Content->functionName == "println"){
            function->add(tac::Call("print_line", 0));
        }
    } else {
        std::string mangled = mangle(functionCall.Content->functionName, functionCall.Content->values);

        int total = 0;

        for(auto& value : functionCall.Content->values){
            Type type = boost::apply_visitor(GetTypeVisitor(), value);   

            if(type.isArray()){
                //Passing an array is just passing an adress
                total += size(BaseType::INT);
            } else {
                total += size(type);
            }
        }

        function->add(tac::Call(mangled, total, return_, return2_));
    }
}

void tac::TacCompiler::compile(ast::Program& program, StringPool& pool, tac::Program& tacProgram) const {
    CompilerVisitor visitor(pool, tacProgram);
    visitor(program);
}
