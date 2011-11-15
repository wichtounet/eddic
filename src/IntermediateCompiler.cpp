//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "IntermediateCompiler.hpp"

#include "MainDeclaration.hpp"
#include "Methods.hpp"
#include "StringPool.hpp"
#include "VisitorUtils.hpp"

#include "Context.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "SemanticalException.hpp"
#include "IsImmediateVisitor.hpp"

#include "mangling.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"
#include "il/Labels.hpp"
#include "il/Math.hpp"

#include "ast/Program.hpp"

using namespace eddic;

inline bool isImmediate(ast::Value& value){
   return boost::apply_visitor(IsImmediateVisitor(), value); 
}

inline Operation toOperation(char op){
    switch(op){
        case '+':
            return Operation::ADD;
        case '-':
            return Operation::SUB;
        case '/':
            return Operation::DIV;
        case '*':
            return Operation::MUL;
        case '%':
            return Operation::MOD;
        default:
            assert(false);
    }
}

inline void putInRegister(ast::Value& value, std::shared_ptr<Operand> operand, IntermediateProgram& program);

inline std::shared_ptr<Operand> performIntOperation(ast::ComposedValue& value, IntermediateProgram& program){
    assert(value.Content->operations.size() > 0); //This has been enforced by previous phases

    auto registerA = program.registers(EAX);
    auto registerB = program.registers(EBX);

    putInRegister(value.Content->first, registerA, program);

    //Apply all the operations in chain
    for(auto& operation : value.Content->operations){
        if(isImmediate(operation.get<1>())){
            putInRegister(operation.get<1>(), registerB, program);

            //Perform the operation 
            program.addInstruction(program.factory().createMath(toOperation(operation.get<0>()), registerA, registerB));
        } else { //The right value is composed
            program.addInstruction(program.factory().createPush(registerA)); //To be sure that the right operation does not override our register 
            
            putInRegister(operation.get<1>(), registerB, program);
            program.addInstruction(program.factory().createMove(createStackOperand(0), registerA));
            
            program.addInstruction(program.factory().createMath(toOperation(operation.get<0>()), registerA, registerB));
        }
    }

    return registerB;
}

inline std::pair<std::shared_ptr<Operand>, std::shared_ptr<Operand>> performStringOperation(ast::ComposedValue& value, IntermediateProgram& program);

class PushValue : public boost::static_visitor<> {
    private:
        IntermediateProgram& program;
    
    public:
        PushValue(IntermediateProgram& p) : program(p) {}

        void operator()(ast::Litteral& litteral){
            program.addInstruction(
                program.factory().createPush(
                    createImmediateOperand(litteral.label)
                )
            );
            
            program.addInstruction(
                program.factory().createPush(
                    createImmediateOperand(litteral.value.size() - 2)
                )
            );
        }

        void operator()(ast::Integer& integer){
            program.addInstruction(
                program.factory().createPush(
                    createImmediateOperand(integer.value)
                )
            );
        }

        void operator()(ast::VariableValue& variable){
            auto var = variable.Content->var;

            if(var->type().base() == BaseType::INT){
                program.addInstruction(
                    program.factory().createPush(
                        var->toIntegerOperand()
                    )
                );
            } else {
                auto operands = var->toStringOperand();

                program.addInstruction(
                    program.factory().createPush(
                        operands.first
                    )
                );
                
                program.addInstruction(
                    program.factory().createPush(
                        operands.second
                    )
                );
            }
        }

        void operator()(ast::ArrayValue& array){
            auto var = array.Content->var;
            auto position = var->position();

            auto registerA = program.registers(EAX);
            auto registerB = program.registers(EBX);

            putInRegister(array.Content->indexValue, registerA, program);
            program.addInstruction(program.factory().createMath(Operation::MUL, createImmediateOperand(size(var->type().base())), registerA));
           
            if(position.isGlobal()){
                program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));
                
                if(var->type().base() == BaseType::INT){
                    program.addInstruction(program.factory().createPush(createValueOfOperand(registerB->getValue())));
                } else {
                    program.addInstruction(program.factory().createPush(createValueOfOperand(registerB->getValue())));
                    program.addInstruction(program.factory().createPush(createValueOfOperand(registerB->getValue(), 4)));
                }
            } else if(position.isStack()) {
                program.addInstruction(program.factory().createMove(createImmediateOperand(position.offset()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));
            
                auto registerEBP = program.registers(EBP);
                program.addInstruction(program.factory().createMath(Operation::ADD, registerEBP, registerB));
                
                if(var->type().base() == BaseType::INT){
                    program.addInstruction(program.factory().createPush(createValueOfOperand(registerB->getValue())));
                } else {
                    program.addInstruction(program.factory().createPush(createValueOfOperand(registerB->getValue())));
                    program.addInstruction(program.factory().createPush(createValueOfOperand(registerB->getValue(), 4)));
                }
            } else if(position.isParameter()) {
                //TODO Implement
            }
        }

        void operator()(ast::ComposedValue& value){
            Type type = GetTypeVisitor()(value);

            if(type.base() == BaseType::INT){
                program.addInstruction(program.factory().createPush(performIntOperation(value, program)));
            } else if(type.base() == BaseType::STRING){
                auto pair = performStringOperation(value, program);

                program.addInstruction(program.factory().createPush(pair.first));
                program.addInstruction(program.factory().createPush(pair.second));
            }
        } 
};

class AssignValueToOperand : public boost::static_visitor<> {
    private:
        std::shared_ptr<Operand> operand;
        IntermediateProgram& program;
    
    public:
        AssignValueToOperand(std::shared_ptr<Operand> op, IntermediateProgram& p) : operand(op), program(p) {}

        void operator()(ast::Litteral&){
            assert(false); //Cannot assign a string to a single operand
        }

        void operator()(ast::Integer& integer){
            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(integer.value),
                    operand
                )
            ); 
        }

        void operator()(ast::VariableValue& variable){
            if(variable.Content->var->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(variable.Content->var->toIntegerOperand(), operand));
            } else {
                assert(false); //Cannot assign a string to a single operand
            }
        }

        void operator()(ast::ArrayValue& array){
            if(array.Content->var->type().base() == BaseType::INT){
                auto var = array.Content->var;
                auto position = var->position();

                auto registerA = program.registers(EAX);
                auto registerB = program.registers(EBX);

                putInRegister(array.Content->indexValue, registerA, program);
                program.addInstruction(program.factory().createMath(Operation::MUL, createImmediateOperand(size(var->type().base())), registerA));

                if(position.isGlobal()){
                    program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), registerB));
                    program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));

                    program.addInstruction(program.factory().createMove(createValueOfOperand(registerB->getValue()), operand));
                } else if(position.isStack()) {
                    program.addInstruction(program.factory().createMove(createImmediateOperand(position.offset()), registerB));
                    program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));
                
                    auto registerEBP = program.registers(EBP);
                    program.addInstruction(program.factory().createMath(Operation::ADD, registerEBP, registerB));
                    
                    program.addInstruction(program.factory().createMove(createValueOfOperand(registerB->getValue()), operand));
                } else if(position.isParameter()){
                    //TODO Implement
                }
            } else {
                assert(false); //Cannot assign a string to a single operand
            }
        }

        void operator()(ast::ComposedValue& value){
            assert(GetTypeVisitor()(value).base() == BaseType::INT); //Cannot be used for string operations

            program.addInstruction(program.factory().createMove(performIntOperation(value, program), operand));
        } 
};

class AssignValueToVariable : public boost::static_visitor<> {
    private:
        std::shared_ptr<Variable> variable;
        IntermediateProgram& program;
    
    public:
        AssignValueToVariable(std::shared_ptr<Variable> v, IntermediateProgram& p) : variable(v), program(p) {}

        void operator()(ast::Litteral& litteral){
            auto operands = variable->toStringOperand();

            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(litteral.label),
                    operands.first
                )
            );
            
            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(litteral.value.size() - 2),
                    operands.second
                )
            );
        }

        void operator()(ast::Integer& integer){
            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(integer.value),
                    variable->toIntegerOperand()
                )
            ); 
        }

        void operator()(ast::VariableValue& variableSource){
            auto var = variableSource.Content->var;

            if(var->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(var->toIntegerOperand(), variable->toIntegerOperand()));
            } else {
                auto source = var->toStringOperand();
                auto destination = variable->toStringOperand();

                program.addInstruction(program.factory().createMove(source.first, destination.first));
                program.addInstruction(program.factory().createMove(source.second, destination.second));
            }
        }

        void operator()(ast::ArrayValue& array){
            auto var = array.Content->var;
            auto position = var->position();

            auto registerA = program.registers(EAX);
            auto registerB = program.registers(EBX);

            putInRegister(array.Content->indexValue, registerA, program);
            program.addInstruction(program.factory().createMath(Operation::MUL, createImmediateOperand(size(var->type().base())), registerA));
           
            if(position.isGlobal()){
                program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));
                
                if(var->type().base() == BaseType::INT){
                    program.addInstruction(program.factory().createMove(createValueOfOperand(registerB->getValue()), variable->toIntegerOperand()));
                } else {
                    auto destination = variable->toStringOperand();
                   
                    program.addInstruction(program.factory().createMove(createValueOfOperand(registerB->getValue()), destination.first));
                    program.addInstruction(program.factory().createMove(createValueOfOperand(registerB->getValue(), 4), destination.second));
                }
            } else if(position.isStack()) {
                program.addInstruction(program.factory().createMove(createImmediateOperand(position.offset()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));

                auto registerEBP = program.registers(EBP);
                program.addInstruction(program.factory().createMath(Operation::ADD, registerEBP, registerB));
                
                if(var->type().base() == BaseType::INT){
                    program.addInstruction(program.factory().createMove(createValueOfOperand(registerB->getValue()), variable->toIntegerOperand()));
                } else {
                    auto destination = variable->toStringOperand();
                   
                    program.addInstruction(program.factory().createMove(createValueOfOperand(registerB->getValue()), destination.first));
                    program.addInstruction(program.factory().createMove(createValueOfOperand(registerB->getValue(), 4), destination.second));
                }
            } else if(position.isParameter()) {
                //TODO Implement
            }
        }

        void operator()(ast::ComposedValue& value){
            Type type = GetTypeVisitor()(value);

            if(type.base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(performIntOperation(value, program), variable->toIntegerOperand()));
            } else if(type.base() == BaseType::STRING){
                auto source = performStringOperation(value, program);
                auto destination = variable->toStringOperand();

                program.addInstruction(program.factory().createMove(source.first, destination.first));
                program.addInstruction(program.factory().createMove(source.second, destination.second));
            }
        } 
};

struct AssignValueToArray : public boost::static_visitor<> {
    private:
        std::shared_ptr<Variable> variable;
        ast::Value indexValue;
        IntermediateProgram& program;

    public:
        AssignValueToArray(std::shared_ptr<Variable> v, ast::Value i, IntermediateProgram& p) : variable(v), indexValue(i), program(p) {}

        void operator()(ast::Litteral& litteral){
            assert(variable->type().base() == BaseType::STRING);

            auto registerA = program.registers(EAX);
            auto registerB = program.registers(EBX);

            putInRegister(indexValue, registerA, program);
            program.addInstruction(program.factory().createMath(Operation::MUL, createImmediateOperand(size(variable->type().base())), registerA));

            auto position = variable->position();
            if(position.isGlobal()){
                program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));

                program.addInstruction(program.factory().createMove(createImmediateOperand(litteral.label), createValueOfOperand(registerB->getValue())));
                program.addInstruction(program.factory().createMove(createImmediateOperand(litteral.value.size() -2), createValueOfOperand(registerB->getValue(), 4)));
            } else if(position.isStack()){
                program.addInstruction(program.factory().createMove(createImmediateOperand(position.offset()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));

                auto registerEBP = program.registers(EBP);
                program.addInstruction(program.factory().createMath(Operation::ADD, registerEBP, registerB));

                program.addInstruction(program.factory().createMove(createImmediateOperand(litteral.label), createValueOfOperand(registerB->getValue())));
                program.addInstruction(program.factory().createMove(createImmediateOperand(litteral.value.size() -2), createValueOfOperand(registerB->getValue(), 4)));
            } else if(position.isParameter()){
                //TODO Implement
            }
        }
        
        void operator()(ast::Integer& integer){
            assert(variable->type().base() == BaseType::INT);

            auto registerA = program.registers(EAX);
            auto registerB = program.registers(EBX);

            putInRegister(indexValue, registerA, program);
            program.addInstruction(program.factory().createMath(Operation::MUL, createImmediateOperand(size(variable->type().base())), registerA));

            auto position = variable->position();
            if(position.isGlobal()){
                program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));

                program.addInstruction(program.factory().createMove(createImmediateOperand(integer.value), createValueOfOperand(registerB->getValue())));
            } else if(position.isStack()){
                program.addInstruction(program.factory().createMove(createImmediateOperand(position.offset()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));

                auto registerEBP = program.registers(EBP);
                program.addInstruction(program.factory().createMath(Operation::ADD, registerEBP, registerB));

                program.addInstruction(program.factory().createMove(createImmediateOperand(integer.value), createValueOfOperand(registerB->getValue())));
            } else if(position.isParameter()){
                //TODO Implement
            }
        }
        
        void operator()(ast::VariableValue& destination){
            auto registerA = program.registers(EAX);
            auto registerB = program.registers(EBX);

            putInRegister(indexValue, registerA, program);
            program.addInstruction(program.factory().createMath(Operation::MUL, createImmediateOperand(size(variable->type().base())), registerA));

            auto position = variable->position();
            if(position.isGlobal()){
                program.addInstruction(program.factory().createMove(createImmediateOperand("VA" + position.name()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));
            } else if(position.isStack()){
                program.addInstruction(program.factory().createMove(createImmediateOperand(position.offset()), registerB));
                program.addInstruction(program.factory().createMath(Operation::ADD, registerA, registerB));

                auto registerEBP = program.registers(EBP);
                program.addInstruction(program.factory().createMath(Operation::ADD, registerEBP, registerB));
            } else if(position.isParameter()){
                //TODO Implement
            }
            
            if(variable->type().base() == BaseType::INT){
                program.addInstruction(program.factory().createMove(destination.Content->var->toIntegerOperand(), createValueOfOperand(registerB->getValue())));
            } else if(variable->type().base() == BaseType::STRING){
                auto operands = destination.Content->var->toStringOperand();

                program.addInstruction(program.factory().createMove(operands.first, createValueOfOperand(registerB->getValue())));
                program.addInstruction(program.factory().createMove(operands.second, createValueOfOperand(registerB->getValue(), 4)));
            }
        }
        
        void operator()(ast::ArrayValue& array){
            //TODO Implement
        }
        
        void operator()(ast::ComposedValue& value){
            //TODO Implement
        }
};

inline JumpCondition toJumpNotCondition(std::string op){
    if(op == "!="){
        return JumpCondition::EQUALS;
    } else if(op == "=="){
        return JumpCondition::NOT_EQUALS;
    } else if(op == ">="){
        return JumpCondition::LESS;
    } else if(op == ">"){
        return JumpCondition::LESS_EQUALS;
    } else if(op == "<="){
        return JumpCondition::GREATER;
    } else if(op == "<"){
        return JumpCondition::GREATER_EQUALS;
    }

    assert(false); //Not handled
}

inline void writeILJumpIfNot(IntermediateProgram& program, ast::Condition& condition, const std::string& label) {
    //No need to jump for a true boolean value 
    if(boost::get<ast::False>(&condition)){
        program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label));
    } else if(auto* ptr = boost::get<ast::BinaryCondition>(&condition)){
        ast::BinaryCondition& binaryCondition = *ptr;
        
        AssignValueToOperand leftVisitor(program.registers(EAX), program);

        boost::apply_visitor(leftVisitor, binaryCondition.Content->lhs);
        
        AssignValueToOperand rightVisitor(program.registers(EBX), program);
        boost::apply_visitor(rightVisitor, binaryCondition.Content->rhs);

        program.addInstruction(program.factory().createCompare(program.registers(EBX), program.registers(EAX)));

        program.addInstruction(program.factory().createJump(toJumpNotCondition(binaryCondition.Content->op), label));
    }
}

inline std::pair<std::shared_ptr<Operand>, std::shared_ptr<Operand>> performStringOperation(ast::ComposedValue& value, IntermediateProgram& program){
    assert(value.Content->operations.size() > 0); //Other values must be transformed before that phase

    auto registerA = program.registers(EAX);
    auto registerB = program.registers(EDX);

    PushValue pusher(program); 
    boost::apply_visitor(pusher, value.Content->first);

    unsigned int iter = 0;

    //Perfom all the additions
    for(auto& operation : value.Content->operations){
        boost::apply_visitor(pusher, operation.get<1>());

        program.addInstruction(program.factory().createCall("concat"));
        
        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(16), program.registers(ESP)));

        ++iter;

        //If there is more operation, push the answer
        if(iter < value.Content->operations.size()){
           program.addInstruction(program.factory().createPush(registerA)); 
           program.addInstruction(program.factory().createPush(registerB)); 
        }
    }
    
    return make_pair(registerA, registerB); 
}

inline void putInRegister(ast::Value& value, std::shared_ptr<Operand> operand, IntermediateProgram& program){
    if(isImmediate(value)){
        AssignValueToOperand visitor(operand, program);
        boost::apply_visitor(visitor, value);
    } else {
        PushValue visitor(program);
        boost::apply_visitor(visitor, value);

        program.addInstruction(program.factory().createMove(createStackOperand(0), operand));

        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(4), program.registers(ESP)));
    }
}

class CompilerVisitor : public boost::static_visitor<> {
    private:
        StringPool& pool;
        IntermediateProgram& program;
    
    public:
        CompilerVisitor(StringPool& p, IntermediateProgram& intermediateProgram) : pool(p), program(intermediateProgram) {}
        
        void operator()(ast::Program& p){
            MainDeclaration().writeIL(program);

            visit_each(*this, p.Content->blocks);

            Methods().writeIL(program);

            pool.writeIL(program);

            p.Content->context->writeIL(program);
        }

        void operator()(ast::FunctionDeclaration& function){
            program.addInstruction(program.factory().createFunctionDeclaration(function.Content->mangledName, function.Content->context->size()));

            //TODO Init arrays

            visit_each(*this, function.Content->instructions);

            program.addInstruction(program.factory().createFunctionExit(function.Content->context->size()));
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
                std::string a = newLabel();

                writeILJumpIfNot(program, if_.Content->condition, a);

                visit_each(*this, if_.Content->instructions);

                if (if_.Content->else_) {
                    std::string b = newLabel();

                    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, b));

                    program.addInstruction(program.factory().createLabel(a));

                    visit_each(*this, (*if_.Content->else_).instructions);

                    program.addInstruction(program.factory().createLabel(b));
                } else {
                    program.addInstruction(program.factory().createLabel(a));
                }
            } else {
                std::string end = newLabel();
                std::string next = newLabel();

                writeILJumpIfNot(program, if_.Content->condition, next);

                visit_each(*this, if_.Content->instructions);

                program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, end));

                for (std::vector<ast::ElseIf>::size_type i = 0; i < if_.Content->elseIfs.size(); ++i) {
                    ast::ElseIf& elseIf = if_.Content->elseIfs[i];

                    program.addInstruction(program.factory().createLabel(next));

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

                    writeILJumpIfNot(program, elseIf.condition, next);

                    visit_each(*this, elseIf.instructions);

                    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, end));
                }

                if (if_.Content->else_) {
                    program.addInstruction(program.factory().createLabel(next));

                    visit_each(*this, (*if_.Content->else_).instructions);
                }

                program.addInstruction(program.factory().createLabel(end));
            }
        }

        void operator()(ast::Assignment& assignment){
            AssignValueToVariable visitor(assignment.Content->context->getVariable(assignment.Content->variableName), program);
            boost::apply_visitor(visitor, assignment.Content->value);
        }
        
        void operator()(ast::ArrayAssignment& assignment){
            AssignValueToArray visitor(assignment.Content->context->getVariable(assignment.Content->variableName), assignment.Content->indexValue, program);
            boost::apply_visitor(visitor, assignment.Content->value);
        }

        void operator()(ast::VariableDeclaration& declaration){
            AssignValueToVariable visitor(declaration.Content->context->getVariable(declaration.Content->variableName), program);
            boost::apply_visitor(visitor, *declaration.Content->value);
        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;

            //We have the guarantee here that both variables are of the same type
            switch (lhs_var->type().base()) {
                case BaseType::INT:{
                    auto registerA = program.registers(EAX);
             
                    auto left = lhs_var->toIntegerOperand();
                    auto right = rhs_var->toIntegerOperand();

                    program.addInstruction(program.factory().createMove(left, registerA));
                    program.addInstruction(program.factory().createMove(right, left));
                    program.addInstruction(program.factory().createMove(registerA, right));

                    break;
                }
                case BaseType::STRING:{
                    auto registerA = program.registers(EAX);
                   
                    auto left = lhs_var->toStringOperand();
                    auto right = rhs_var->toStringOperand();
                    
                    program.addInstruction(program.factory().createMove(left.first, registerA));
                    program.addInstruction(program.factory().createMove(right.first, left.first));
                    program.addInstruction(program.factory().createMove(registerA, right.first));
                    
                    program.addInstruction(program.factory().createMove(left.second, registerA));
                    program.addInstruction(program.factory().createMove(right.second, left.second));
                    program.addInstruction(program.factory().createMove(registerA, right.second));
                    
                    break;
                }
                default:
                   throw SemanticalException("Variable of invalid type");
            }
        }

        void operator()(ast::While& while_){
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            program.addInstruction(program.factory().createLabel(startLabel));

            writeILJumpIfNot(program, while_.Content->condition, endLabel);

            visit_each(*this, while_.Content->instructions);

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, startLabel));

            program.addInstruction(program.factory().createLabel(endLabel));
        }

        void operator()(ast::For for_){
            visit_optional(*this, for_.Content->start);

            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            program.addInstruction(program.factory().createLabel(startLabel));

            if(for_.Content->condition){
                writeILJumpIfNot(program, *for_.Content->condition, endLabel);
            }

            visit_each(*this, for_.Content->instructions);

            visit_optional(*this, for_.Content->repeat);

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, startLabel));

            program.addInstruction(program.factory().createLabel(endLabel));
        }

        //TODO Rewrite that function, perhaps with a transformation into several element in a previous stage
        void operator()(ast::Foreach& foreach){
            ast::Integer fromValue;
            fromValue.value = foreach.Content->from;
            
            ast::Integer toValue;
            toValue.value = foreach.Content->to;

            AssignValueToVariable visitor(foreach.Content->context->getVariable(foreach.Content->variableName), program);
            
            //Assign the base value to the variable
            visit_non_variant(visitor, fromValue);
            
            std::string startLabel = newLabel();
            std::string endLabel = newLabel();

            program.addInstruction(program.factory().createLabel(startLabel));

            //Create a condition
            ast::VariableValue v;
            v.Content->variableName = foreach.Content->variableName;
            v.Content->context = foreach.Content->context;
            v.Content->var = v.Content->context->getVariable(foreach.Content->variableName);
        
            //Avoid doing all that conversion stuff...  
            ast::Condition condition; 
            ast::BinaryCondition binaryCondition; 
            binaryCondition.Content->lhs = v;
            binaryCondition.Content->rhs = toValue;
            binaryCondition.Content->op = "<=";

            condition = binaryCondition;

            writeILJumpIfNot(program, condition, endLabel);

            //Write all the instructions
            visit_each(*this, foreach.Content->instructions);

            //Increment the variable
            ast::Integer inc;
            inc.value = 1;
           
            ast::ComposedValue addition;
            addition.Content->first = v;
            addition.Content->operations.push_back(boost::tuples::tuple<char, ast::Value>('+', inc));
           
            visit_non_variant(visitor, addition);
            
            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, startLabel));

            program.addInstruction(program.factory().createLabel(endLabel));
        }

        void operator()(ast::FunctionCall& functionCall){
            PushValue visitor(program);
            for(auto& value : functionCall.Content->values){
                boost::apply_visitor(visitor, value);
            }

            if(functionCall.Content->functionName == "print" || functionCall.Content->functionName == "println"){
                Type type = boost::apply_visitor(GetTypeVisitor(), functionCall.Content->values[0]);

                switch (type.base()) {
                    case BaseType::INT:
                        program.addInstruction(program.factory().createCall("print_integer"));
                        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(4), program.registers(ESP)));

                        break;
                    case BaseType::STRING:
                        program.addInstruction(program.factory().createCall("print_string"));
                        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(8), program.registers(ESP)));

                        break;
                    default:
                        throw SemanticalException("Variable of invalid type");
                }
    
                if(functionCall.Content->functionName == "println"){
                    program.addInstruction(program.factory().createCall("print_line"));
                }
            } else {
                std::string mangled = mangle(functionCall.Content->functionName, functionCall.Content->values);

                program.addInstruction(program.factory().createCall(mangled));

                int total = 0;

                for(auto& value : functionCall.Content->values){
                    Type type = boost::apply_visitor(GetTypeVisitor(), value);   

                    total += size(type);
                }

                program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(total), program.registers(ESP)));
            }
        }
};

void IntermediateCompiler::compile(ast::Program& program, StringPool& pool, IntermediateProgram& intermediateProgram){
    CompilerVisitor visitor(pool, intermediateProgram);
    visitor(program);
}
