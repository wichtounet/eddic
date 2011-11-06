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
#include "Variable.hpp"
#include "SemanticalException.hpp"

#include "mangling.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"
#include "il/Labels.hpp"

#include "ast/Program.hpp"

using namespace eddic;

class PushValue : public boost::static_visitor<> {
    private:
        IntermediateProgram& program;
    
    public:
        PushValue(IntermediateProgram& p) : program(p) {}

        void operator()(ASTLitteral& litteral){
            //TODO
        }

        void operator()(ASTInteger& integer){
            //TODO
        }

        void operator()(ASTVariable& variable){
            //TODO
        }

        void operator()(ASTComposedValue& value){
            //TODO
        } 
};

class AssignValueToVariable : public boost::static_visitor<> {
    private:
        std::shared_ptr<Variable> var;
        IntermediateProgram& program;
    
    public:
        AssignValueToVariable(std::shared_ptr<Variable> v, IntermediateProgram& p) : var(v), program(p) {}

        void operator()(ASTLitteral& litteral){
            //TODO
        }

        void operator()(ASTInteger& integer){
            //TODO
        }

        void operator()(ASTVariable& variable){
            //TODO
        }

        void operator()(ASTComposedValue& value){
            //TODO
        } 
};

class AssignValueToOperand : public boost::static_visitor<> {
    private:
        std::shared_ptr<Operand> operand;
        IntermediateProgram& program;
    
    public:
        AssignValueToOperand(std::shared_ptr<Operand> op, IntermediateProgram& p) : operand(op), program(p) {}

        void operator()(ASTLitteral& litteral){
            //TODO
        }

        void operator()(ASTInteger& integer){
            //TODO
        }

        void operator()(ASTVariable& variable){
            //TODO
        }

        void operator()(ASTComposedValue& value){
            //TODO
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

void writeILJumpIfNot(IntermediateProgram& program, ASTCondition& condition, const std::string& label, int labelIndex) {
    //No need to jump for a true boolean value 
    if(boost::get<ASTFalse>(&condition)){
        program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label(label, labelIndex)));
    } else if(auto* ptr = boost::get<ASTBinaryCondition>(&condition)){
        JumpIfNot visitor(program, eddic::label(label, labelIndex));
        boost::apply_visitor(visitor, *ptr);
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

        void operator()(GlobalVariableDeclaration& variable){
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
            addition.first = inc;
            addition.operations.push_back(boost::tuples::tuple<char, ASTValue>('+', v));
           
            visit_non_variant(visitor, addition);
            
            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label("start_foreach", labels)));

            program.addInstruction(program.factory().createLabel(label("end_foreach", labels)));
        }

        void operator()(ASTFunctionCall& functionCall){
            PushValue visitor(program);
            for(auto& value : functionCall.values){
                boost::apply_visitor(visitor, value);
            }

            std::string mangled = mangle(functionCall.functionName, functionCall.values);
            program.addInstruction(program.factory().createCall(mangled));
        }
};

void IntermediateCompiler::compile(ASTProgram& program, StringPool& pool, IntermediateProgram& intermediateProgram){
    CompilerVisitor visitor(pool, intermediateProgram);
    visitor(program);
}
