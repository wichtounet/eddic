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
#include "IsConstantVisitor.hpp"
#include "mangling.hpp"

#include "tac/TacCompiler.hpp"
#include "tac/Program.hpp"

#include "ast/Program.hpp"

//TODO Move label generator system in another folder
#include "il/Labels.hpp"

using namespace eddic;

namespace {

struct IsSingleArgumentVisitor : public boost::static_visitor<bool> {
    bool operator()(ast::VariableValue&) const {
        return true;
    }

    bool operator()(ast::Integer&) const {
        return true;
    }
    
    bool operator()(ast::Litteral&) const {
        return false;
    }

    bool operator()(ast::ArrayValue&) const {
        return false;
    }

    bool operator()(ast::ComposedValue&) const {
        return false;
    }

    //A call to a function returning an int is single argument
    bool operator()(ast::FunctionCall& call) const {
        Type type = call.Content->function->returnType;

        return type.base() == BaseType::INT;
    }
};

struct IsParamSafeVisitor : public boost::static_visitor<bool> {
    bool operator()(ast::VariableValue&) const {
        return true;
    }

    bool operator()(ast::Integer&) const {
        return true;
    }
    
    bool operator()(ast::Litteral&) const {
        return true;
    }

    bool operator()(ast::ArrayValue&) const {
        return false;//TODO In some cases, it's possible that it can be param safe
    }

    bool operator()(ast::ComposedValue&) const {
        return false;//TODO In some cases, it's possible that it can be param safe
    }

    bool operator()(ast::FunctionCall&) const {
        return false;
    }
};

void performStringOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2);
void executeCall(ast::FunctionCall& functionCall, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);
tac::Argument moveToArgument(ast::Value& value, std::shared_ptr<tac::Function> function);

void performIntOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> variable){
    assert(value.Content->operations.size() > 0); //This has been enforced by previous phases

    tac::Argument left = moveToArgument(value.Content->first, function);
    tac::Argument right;

    auto t1 = function->context->newTemporary(); 

    //Apply all the operations in chain
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        right = moveToArgument(operation.get<1>(), function);
       
        if(i == value.Content->operations.size() - 1){
            function->add(std::make_shared<tac::Quadruple>(variable, left, tac::toOperator(operation.get<0>()), right));
        } else if (i == 0){
            function->add(std::make_shared<tac::Quadruple>(t1, left, tac::toOperator(operation.get<0>()), right));
        } else {
            function->add(std::make_shared<tac::Quadruple>(t1, t1, tac::toOperator(operation.get<0>()), right));
        }
    }
}

tac::Argument computeIndexOfArray(std::shared_ptr<Variable> array, tac::Argument index, std::shared_ptr<tac::Function> function){
    auto temp = function->context->newTemporary();
    auto position = array->position();

    if(position.isGlobal()){
        function->add(std::make_shared<tac::Quadruple>(temp, index, tac::Operator::MUL, -1 * size(array->type().base())));
        function->add(std::make_shared<tac::Quadruple>(temp, temp, tac::Operator::ADD, (size(array->type().base()) * array->type().size())));
        function->add(std::make_shared<tac::Quadruple>(temp, temp, tac::Operator::SUB, size(BaseType::INT)));
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

struct ToArgumentsVisitor : public boost::static_visitor<std::vector<tac::Argument>> {
    ToArgumentsVisitor(std::shared_ptr<tac::Function> f) : function(f) {}
    
    mutable std::shared_ptr<tac::Function> function;

    std::vector<tac::Argument> operator()(ast::Litteral& litteral) const {
        return {litteral.label, litteral.value.size() - 2};
    }

    std::vector<tac::Argument> operator()(ast::Integer& integer) const {
        return {integer.value};
    }

    std::vector<tac::Argument> operator()(ast::FunctionCall& call) const {
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

    std::vector<tac::Argument> operator()(ast::VariableValue& value) const {
        auto type = value.Content->var->type();

        if(type.base() == BaseType::INT){
            return {value.Content->var};
        } else {
            auto temp = value.Content->context->newTemporary();
            function->add(std::make_shared<tac::Quadruple>(temp, value.Content->var, tac::Operator::DOT, -4));
            
            return {value.Content->var, temp};
        }
    }

    std::vector<tac::Argument> operator()(ast::ArrayValue& array) const {
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

    std::vector<tac::Argument> operator()(ast::ComposedValue& value) const {
        Type type = GetTypeVisitor()(value);

        if(type.base() == BaseType::INT){
            auto t1 = function->context->newTemporary();
            performIntOperation(value, function, t1);

            return {t1};
        } else {
            auto t1 = function->context->newTemporary();
            auto t2 = function->context->newTemporary();

            performStringOperation(value, function, t1, t2);
            
            return {t1, t2};
        }
    }
};

struct AbstractVisitor : public boost::static_visitor<> {
    AbstractVisitor(std::shared_ptr<tac::Function> f) : function(f) {}
    
    mutable std::shared_ptr<tac::Function> function;
    
    virtual void intAssign(std::vector<tac::Argument> arguments) const = 0;
    virtual void stringAssign(std::vector<tac::Argument> arguments) const = 0;
    
    void operator()(ast::Litteral& litteral) const {
        stringAssign(ToArgumentsVisitor(function)(litteral));
    }

    void operator()(ast::Integer& integer) const {
        intAssign(ToArgumentsVisitor(function)(integer));
    }

    void operator()(ast::FunctionCall& call) const {
        Type type = call.Content->function->returnType;

        if(type.base() == BaseType::INT){
            intAssign(ToArgumentsVisitor(function)(call));
        } else if(type.base() == BaseType::STRING){
            stringAssign(ToArgumentsVisitor(function)(call));
        } else {
            throw SemanticalException("This function doesn't return anything");   
        }
    }

    void operator()(ast::VariableValue& value) const {
        auto type = value.Content->var->type();

        if(type.base() == BaseType::INT){
            intAssign(ToArgumentsVisitor(function)(value));
        } else if(type.base() == BaseType::STRING){
            stringAssign(ToArgumentsVisitor(function)(value));
        } else {
            throw SemanticalException("Invalid variable type");   
        }
    }

    void operator()(ast::ArrayValue& array) const {
        if(array.Content->var->type().base() == BaseType::INT){
            intAssign(ToArgumentsVisitor(function)(array));
        } else if(array.Content->var->type().base() == BaseType::STRING){
            stringAssign(ToArgumentsVisitor(function)(array));
        } else {
            throw SemanticalException("Invalid variable type");   
        }
    }

    void operator()(ast::ComposedValue& value) const {
        Type type = GetTypeVisitor()(value);
        
        if(type.base() == BaseType::INT){
            intAssign(ToArgumentsVisitor(function)(value));
        } else if(type.base() == BaseType::STRING){
            stringAssign(ToArgumentsVisitor(function)(value));
        } else {
            throw SemanticalException("Invalid variable type");   
        }
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
        function->add(std::make_shared<tac::Quadruple>(variable, -4, tac::Operator::DOT_ASSIGN, arguments[1]));
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
        function->add(std::make_shared<tac::Goto>(label));
    }
   
    void operator()(ast::BinaryCondition& binaryCondition) const {
        tac::Argument left = moveToArgument(binaryCondition.Content->lhs, function);
        tac::Argument right = moveToArgument(binaryCondition.Content->rhs, function);

        function->add(std::make_shared<tac::IfFalse>(tac::toBinaryOperator(binaryCondition.Content->op), left, right, label));
    }
};

void performStringOperation(ast::ComposedValue& value, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> v1, std::shared_ptr<Variable> v2){
    assert(value.Content->operations.size() > 0); //Other values must be transformed before that phase

    std::vector<tac::Argument> arguments;

    auto first = boost::apply_visitor(ToArgumentsVisitor(function), value.Content->first);
    arguments.insert(arguments.end(), first.begin(), first.end());

    //Perfom all the additions
    for(unsigned int i = 0; i < value.Content->operations.size(); ++i){
        auto operation = value.Content->operations[i];

        auto second = boost::apply_visitor(ToArgumentsVisitor(function), operation.get<1>());
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
        
        void operator()(ast::Program& p){
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

                boost::apply_visitor(JumpIfFalseVisitor(function, endLabel), if_.Content->condition);

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

                boost::apply_visitor(JumpIfFalseVisitor(function, next), if_.Content->condition);

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

                    boost::apply_visitor(JumpIfFalseVisitor(function, next), elseIf.condition);

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
            
            auto temp = swap.Content->context->newTemporary();

            if(lhs_var->type().base() == BaseType::INT || lhs_var->type().base() == BaseType::STRING){
                function->add(std::make_shared<tac::Quadruple>(temp, rhs_var));  
                function->add(std::make_shared<tac::Quadruple>(rhs_var, lhs_var));  
                function->add(std::make_shared<tac::Quadruple>(lhs_var, temp));  
                
                if( lhs_var->type().base() == BaseType::STRING){
                    function->add(std::make_shared<tac::Quadruple>(temp, rhs_var, tac::Operator::DOT, -4));  
                    function->add(std::make_shared<tac::Quadruple>(rhs_var, lhs_var, tac::Operator::DOT, -4));  
                    function->add(std::make_shared<tac::Quadruple>(lhs_var, temp));  
                }
            } else {
                throw SemanticalException("Variable of invalid type");
            }
        }

        void operator()(ast::While& while_){
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            function->add(startLabel);

            boost::apply_visitor(JumpIfFalseVisitor(function, endLabel), while_.Content->condition);

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
                boost::apply_visitor(JumpIfFalseVisitor(function, endLabel), *for_.Content->condition);
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
                function->add(std::make_shared<tac::Quadruple>(var, -4, tac::Operator::DOT_ASSIGN, stringTemp));
            }

            visit_each(*this, foreach.Content->instructions);    

            function->add(std::make_shared<tac::Quadruple>(iterVar, iterVar, tac::Operator::ADD, 1)); 

            function->add(std::make_shared<tac::Goto>(startLabel));
           
            function->add(endLabel); 
        }

        void operator()(ast::FunctionCall& functionCall){
            executeCall(functionCall, function, {}, {});
        }

        void operator()(ast::Return& return_){
            auto arguments = boost::apply_visitor(ToArgumentsVisitor(function), return_.Content->value);

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
    tac::Argument arg;

    if(boost::apply_visitor(IsSingleArgumentVisitor(), value)){
        arg = boost::apply_visitor(ToArgumentsVisitor(function), value)[0];
    } else {
        auto t1 = function->context->newTemporary();
        boost::apply_visitor(AssignValueToVariable(function, t1), value);
        arg = t1; 
    }
    
    return arg;
}

void executeCall(ast::FunctionCall& functionCall, std::shared_ptr<tac::Function> function, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_){
    std::vector<std::vector<tac::Argument>> arguments;

    for(auto& value : functionCall.Content->values){
        arguments.push_back(boost::apply_visitor(ToArgumentsVisitor(function), value)); 
    }

    for(auto& first : arguments){
        for(auto& arg : first){
            function->add(std::make_shared<tac::Quadruple>(tac::Operator::PARAM, arg));   
        }
    }

    std::string functionName;  
    if(functionCall.Content->functionName == "print" || functionCall.Content->functionName == "println"){
        Type type = boost::apply_visitor(GetTypeVisitor(), functionCall.Content->values[0]);

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
        Type type = boost::apply_visitor(GetTypeVisitor(), value);   

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

void tac::TacCompiler::compile(ast::Program& program, StringPool& pool, tac::Program& tacProgram) const {
    CompilerVisitor visitor(pool, tacProgram);
    visitor(program);
}
