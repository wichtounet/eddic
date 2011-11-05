//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "IntermediateCompiler.hpp"

#include "MainDeclaration.hpp"
#include "Methods.hpp"
#include "StringPool.hpp"
#include "VisitorUtils.hpp"

#include "Context.hpp"
#include "Variable.hpp"
#include "SemanticalException.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"

#include "ast/Program.hpp"

using namespace eddic;

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
            //TODO
        }

        void operator()(ASTAssignment& assignment){
            //TODO
        }

        void operator()(ASTDeclaration& declaration){
            //TODO
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

        void operator()(ASTFor for_){
            //TODO
        }

        void operator()(ASTWhile& while_){
            //TODO
        }

        void operator()(ASTForeach& foreach){
            //TODO
        }

        void operator()(ASTFunctionCall& functionCall){
            //TODO
        }
};

void IntermediateCompiler::compile(ASTProgram& program, StringPool& pool, IntermediateProgram& intermediateProgram){
    CompilerVisitor visitor(pool, intermediateProgram);
    visitor(program);
}
