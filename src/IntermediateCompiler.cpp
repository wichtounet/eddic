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

inline bool isImmediate(ASTValue& value){
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

inline void putInRegister(ASTValue& value, std::shared_ptr<Operand> operand, IntermediateProgram& program);

inline std::shared_ptr<Operand> performIntOperation(ASTComposedValue& value, IntermediateProgram& program){
    assert(value.operations.size() > 0); //This has been enforced by previous phases

    auto registerA = createRegisterOperand("eax");
    auto registerB = createRegisterOperand("ebx");

    putInRegister(value.first, registerA, program);

    //Apply all the operations in chain
    for(auto& operation : value.operations){
        putInRegister(operation.get<1>(), registerB, program);

        //Perform the operation 
        program.addInstruction(program.factory().createMath(toOperation(operation.get<0>()), registerB, registerA));
    }

    return registerA;
}

inline std::pair<std::shared_ptr<Operand>, std::shared_ptr<Operand>> performStringOperation(ASTComposedValue& value, IntermediateProgram& program);

class PushValue : public boost::static_visitor<> {
    private:
        IntermediateProgram& program;
    
    public:
        PushValue(IntermediateProgram& p) : program(p) {}

        void operator()(ASTLitteral& litteral){
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

        void operator()(ASTInteger& integer){
            program.addInstruction(
                program.factory().createPush(
                    createImmediateOperand(integer.value)
                )
            );
        }

        void operator()(ASTVariable& variable){
            auto var = variable.var;

            if(var->type() == Type::INT){
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

        void operator()(ASTComposedValue& value){
            Type type = GetTypeVisitor()(value);

            if(type == Type::INT){
                program.addInstruction(program.factory().createPush(performIntOperation(value, program)));
            } else if(type == Type::STRING){
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

        void operator()(ASTLitteral&){
            assert(false); //Cannot assign a string to a single operand
        }

        void operator()(ASTInteger& integer){
            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(integer.value),
                    operand
                )
            ); 
        }

        void operator()(ASTVariable& variable){
            if(variable.var->type() == Type::INT){
                program.addInstruction(program.factory().createMove(variable.var->toIntegerOperand(), operand));
            } else {
                assert(false); //Cannot assign a string to a single operand
            }
        }

        void operator()(ASTComposedValue& value){
            assert(GetTypeVisitor()(value) == Type::INT); //Cannot be used for string operations

            program.addInstruction(program.factory().createMove(performIntOperation(value, program), operand));
        } 
};

class AssignValueToVariable : public boost::static_visitor<> {
    private:
        std::shared_ptr<Variable> variable;
        IntermediateProgram& program;
    
    public:
        AssignValueToVariable(std::shared_ptr<Variable> v, IntermediateProgram& p) : variable(v), program(p) {}

        void operator()(ASTLitteral& litteral){
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

        void operator()(ASTInteger& integer){
            program.addInstruction(
                program.factory().createMove(
                    createImmediateOperand(integer.value),
                    variable->toIntegerOperand()
                )
            ); 
        }

        void operator()(ASTVariable& variableSource){
            auto var = variableSource.var;

            if(var->type() == Type::INT){
                program.addInstruction(program.factory().createMove(var->toIntegerOperand(), variable->toIntegerOperand()));
            } else {
                auto source = var->toStringOperand();
                auto destination = variable->toStringOperand();

                program.addInstruction(program.factory().createMove(source.first, destination.first));
                program.addInstruction(program.factory().createMove(source.second, destination.second));
            }
        }

        void operator()(ASTComposedValue& value){
            Type type = GetTypeVisitor()(value);

            if(type == Type::INT){
                program.addInstruction(program.factory().createMove(performIntOperation(value, program), variable->toIntegerOperand()));
            } else if(type == Type::STRING){
                auto source = performStringOperation(value, program);
                auto destination = variable->toStringOperand();

                program.addInstruction(program.factory().createMove(source.first, destination.first));
                program.addInstruction(program.factory().createMove(source.second, destination.second));
            }
        } 
};

class JumpIfNot : public boost::static_visitor<> {
    private:
        IntermediateProgram& program;
        std::string label;
    
    public:
        JumpIfNot(IntermediateProgram& p, const std::string& l) : program(p), label(l) {}

        template<typename T>
        void common(T& condition){
            AssignValueToOperand leftVisitor(createRegisterOperand("eax"), program);
            boost::apply_visitor(leftVisitor, condition.lhs);
            
            AssignValueToOperand rightVisitor(createRegisterOperand("ebx"), program);
            boost::apply_visitor(rightVisitor, condition.rhs);

            program.addInstruction(program.factory().createCompare(createRegisterOperand("ebx"), createRegisterOperand("eax")));
        }
        
        void operator()(ASTEquals& equals){
            common(equals);

            program.addInstruction(program.factory().createJump(JumpCondition::NOT_EQUALS, label));
        }
        
        void operator()(ASTNotEquals& notEquals){
            common(notEquals);

            program.addInstruction(program.factory().createJump(JumpCondition::EQUALS, label));
        }
        
        void operator()(ASTLess& less){
            common(less);

            program.addInstruction(program.factory().createJump(JumpCondition::GREATER_EQUALS, label));
        }
        
        void operator()(ASTLessEquals& less){
            common(less);

            program.addInstruction(program.factory().createJump(JumpCondition::GREATER, label));
        }
        
        void operator()(ASTGreater& greater){
            common(greater);

            program.addInstruction(program.factory().createJump(JumpCondition::LESS_EQUALS, label));
        }
        
        void operator()(ASTGreaterEquals& greater){
            common(greater);

            program.addInstruction(program.factory().createJump(JumpCondition::LESS, label));
        }
};

inline void writeILJumpIfNot(IntermediateProgram& program, ASTCondition& condition, const std::string& label, int labelIndex) {
    //No need to jump for a true boolean value 
    if(boost::get<ASTFalse>(&condition)){
        program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label(label, labelIndex)));
    } else if(auto* ptr = boost::get<ASTBinaryCondition>(&condition)){
        JumpIfNot visitor(program, eddic::label(label, labelIndex));
        boost::apply_visitor(visitor, *ptr);
    }
}

inline std::pair<std::shared_ptr<Operand>, std::shared_ptr<Operand>> performStringOperation(ASTComposedValue& value, IntermediateProgram& program){
    assert(value.operations.size() > 0); //Other values must be transformed before that phase

    auto registerA = createRegisterOperand("eax");
    auto registerB = createRegisterOperand("edx");

    PushValue pusher(program); 
    boost::apply_visitor(pusher, value.first);

    unsigned int iter = 0;

    //Perfom all the additions
    for(auto& operation : value.operations){
        boost::apply_visitor(pusher, operation.get<1>());

        program.addInstruction(program.factory().createCall("concat"));
        
        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(16), createRegisterOperand("esp")));

        ++iter;

        //If there is more operation, push the answer
        if(iter < value.operations.size()){
           program.addInstruction(program.factory().createPush(registerA)); 
           program.addInstruction(program.factory().createPush(registerB)); 
        }
    }
    
    return make_pair(registerA, registerB); 
}

inline void putInRegister(ASTValue& value, std::shared_ptr<Operand> operand, IntermediateProgram& program){
    if(isImmediate(value)){
        AssignValueToOperand visitor(operand, program);
        boost::apply_visitor(visitor, value);
    } else {
        PushValue visitor(program);
        boost::apply_visitor(visitor, value);

        program.addInstruction(program.factory().createMove(createStackOperand(0), operand));

        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(4), createRegisterOperand("esp")));
    }
}

class CompilerVisitor : public boost::static_visitor<> {
    private:
        StringPool& pool;
        IntermediateProgram& program;
    
    public:
        CompilerVisitor(StringPool& p, IntermediateProgram& intermediateProgram) : pool(p), program(intermediateProgram) {}
        
        void operator()(ASTProgram& p){
            MainDeclaration().writeIL(program);

            visit_each(*this, p.blocks);

            Methods().writeIL(program);

            pool.writeIL(program);

            p.context->writeIL(program);
        }

        void operator()(ASTFunctionDeclaration& function){
            program.addInstruction(program.factory().createFunctionDeclaration(function.mangledName, function.context->size()));

            visit_each(*this, function.instructions);

            program.addInstruction(program.factory().createFunctionExit(function.context->size()));
        }

        void operator()(GlobalVariableDeclaration&){
            //Nothing to compile, the global variable values are written using global contexts
        }

        void operator()(ASTIf& if_){
            //TODO Make something accessible for others operations
            static int labels = 0;

            if (if_.elseIfs.empty()) {
                int a = labels++;

                writeILJumpIfNot(program, if_.condition, "L", a);

                visit_each(*this, if_.instructions);

                if (if_.else_) {
                    int b = labels++;

                    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label("L", b)));

                    program.addInstruction(program.factory().createLabel(eddic::label("L", a)));

                    visit_each(*this, (*if_.else_).instructions);

                    program.addInstruction(program.factory().createLabel(eddic::label("L", b)));
                } else {
                    program.addInstruction(program.factory().createLabel(eddic::label("L", a)));
                }
            } else {
                int end = labels++;
                int next = labels++;

                writeILJumpIfNot(program, if_.condition, "L", next);

                visit_each(*this, if_.instructions);

                program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label("L", end)));

                for (std::vector<ASTElseIf>::size_type i = 0; i < if_.elseIfs.size(); ++i) {
                    ASTElseIf& elseIf = if_.elseIfs[i];

                    program.addInstruction(program.factory().createLabel(eddic::label("L", next)));

                    //Last elseif
                    if (i == if_.elseIfs.size() - 1) {
                        if (if_.else_) {
                            next = labels++;
                        } else {
                            next = end;
                        }
                    } else {
                        next = labels++;
                    }

                    writeILJumpIfNot(program, elseIf.condition, "L", next);

                    visit_each(*this, elseIf.instructions);

                    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label("L", end)));
                }

                if (if_.else_) {
                    program.addInstruction(program.factory().createLabel(eddic::label("L", next)));

                    visit_each(*this, (*if_.else_).instructions);
                }

                program.addInstruction(program.factory().createLabel(eddic::label("L", end)));
            }
        }

        void operator()(ASTAssignment& assignment){
            AssignValueToVariable visitor(assignment.context->getVariable(assignment.variableName), program);
            boost::apply_visitor(visitor, assignment.value);
        }

        void operator()(ASTDeclaration& declaration){
            AssignValueToVariable visitor(declaration.context->getVariable(declaration.variableName), program);
            boost::apply_visitor(visitor, declaration.value);
        }

        void operator()(ASTSwap& swap){
            //We have the guarantee here that both variables are of the same type
            switch (swap.lhs_var->type()) {
                case Type::INT:{
                    auto registerA = createRegisterOperand("eax");
                    auto registerB = createRegisterOperand("ebx");
               
                    program.addInstruction(program.factory().createMove(swap.lhs_var->toIntegerOperand(), registerA));
                    program.addInstruction(program.factory().createMove(swap.rhs_var->toIntegerOperand(), registerB));

                    program.addInstruction(program.factory().createMove(registerB, swap.lhs_var->toIntegerOperand()));
                    program.addInstruction(program.factory().createMove(registerA, swap.rhs_var->toIntegerOperand()));

                    break;
                }
                case Type::STRING:{
                    auto registerA = createRegisterOperand("eax");
                    auto registerB = createRegisterOperand("ebx");
                    auto registerC = createRegisterOperand("ecx");
                    auto registerD = createRegisterOperand("edx");
                    
                    program.addInstruction(program.factory().createMove(swap.lhs_var->toStringOperand().first, registerA));
                    program.addInstruction(program.factory().createMove(swap.lhs_var->toStringOperand().second, registerB));
                    program.addInstruction(program.factory().createMove(swap.rhs_var->toStringOperand().first, registerC));
                    program.addInstruction(program.factory().createMove(swap.rhs_var->toStringOperand().second, registerD));
                    
                    program.addInstruction(program.factory().createMove(registerC, swap.lhs_var->toStringOperand().first));
                    program.addInstruction(program.factory().createMove(registerD, swap.lhs_var->toStringOperand().second));
                    program.addInstruction(program.factory().createMove(registerA, swap.rhs_var->toStringOperand().first));
                    program.addInstruction(program.factory().createMove(registerB, swap.rhs_var->toStringOperand().second));

                    break;
                }
                default:
                   throw SemanticalException("Variable of invalid type");
            }
        }

        void operator()(ASTWhile& while_){
            //TODO Make something accessible for others operations
            static int labels = 0;

            int startLabel = labels++;
            int endLabel = labels++;

            program.addInstruction(program.factory().createLabel(label("WL", startLabel)));

            writeILJumpIfNot(program, while_.condition, "WL", endLabel);

            visit_each(*this, while_.instructions);

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label("WL", startLabel)));

            program.addInstruction(program.factory().createLabel(label("WL", endLabel)));
        }

        void operator()(ASTFor for_){
            visit_optional(*this, for_.start);

            static int labels = -1;

            ++labels;

            program.addInstruction(program.factory().createLabel(label("start_for", labels)));

            if(for_.condition){
                writeILJumpIfNot(program, *for_.condition, "end_for", labels);
            }

            visit_each(*this, for_.instructions);

            visit_optional(*this, for_.repeat);

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label("start_for", labels)));

            program.addInstruction(program.factory().createLabel(label("end_for", labels)));
        }

        //TODO Rewrite that function, perhaps with a transformation into several element in a previous stage
        void operator()(ASTForeach& foreach){
            ASTInteger fromValue;
            fromValue.value = foreach.from;
            
            ASTInteger toValue;
            toValue.value = foreach.to;

            AssignValueToVariable visitor(foreach.context->getVariable(foreach.variableName), program);
            
            //Assign the base value to the variable
            visit_non_variant(visitor, fromValue);
            
            static int labels = -1;

            ++labels;

            program.addInstruction(program.factory().createLabel(label("start_foreach", labels)));

            //Create a condition
            ASTVariable v;
            v.variableName = foreach.variableName;
            v.context = foreach.context;
            v.var = v.context->getVariable(foreach.variableName);
        
            //Avoid doing all that conversion stuff...  
            ASTCondition condition; 
            ASTBinaryCondition binaryCondition; 
            ASTLessEquals lessEquals;
            lessEquals.lhs = v;
            lessEquals.rhs = toValue;

            binaryCondition = lessEquals;
            condition = binaryCondition;

            writeILJumpIfNot(program, condition, "end_foreach", labels);

            //Write all the instructions
            visit_each(*this, foreach.instructions);

            //Increment the variable
            ASTInteger inc;
            inc.value = 1;
           
            ASTComposedValue addition;
            addition.first = v;
            addition.operations.push_back(boost::tuples::tuple<char, ASTValue>('+', inc));
           
            visit_non_variant(visitor, addition);
            
            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label("start_foreach", labels)));

            program.addInstruction(program.factory().createLabel(label("end_foreach", labels)));
        }

        void operator()(ASTFunctionCall& functionCall){
            PushValue visitor(program);
            for(auto& value : functionCall.values){
                boost::apply_visitor(visitor, value);
            }

            if(functionCall.functionName == "print" || functionCall.functionName == "println"){
                Type type = boost::apply_visitor(GetTypeVisitor(), functionCall.values[0]);

                switch (type) {
                    case Type::INT:
                        program.addInstruction(program.factory().createCall("print_integer"));
                        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(4), createRegisterOperand("esp")));

                        break;
                    case Type::STRING:
                        program.addInstruction(program.factory().createCall("print_string"));
                        program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(8), createRegisterOperand("esp")));

                        break;
                    default:
                        throw SemanticalException("Variable of invalid type");
                }
    
                if(functionCall.functionName == "println"){
                    program.addInstruction(program.factory().createCall("print_line"));
                }
            } else {
                std::string mangled = mangle(functionCall.functionName, functionCall.values);

                program.addInstruction(program.factory().createCall(mangled));

                int total = 0;

                for(auto& value : functionCall.values){
                    Type type = boost::apply_visitor(GetTypeVisitor(), value);   

                    total += size(type);
                }

                program.addInstruction(program.factory().createMath(Operation::ADD, createImmediateOperand(total), createRegisterOperand("esp")));
            }
        }
};

void IntermediateCompiler::compile(ASTProgram& program, StringPool& pool, IntermediateProgram& intermediateProgram){
    CompilerVisitor visitor(pool, intermediateProgram);
    visitor(program);
}
