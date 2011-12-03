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

#include "tac/TacCompiler.hpp"
#include "tac/Program.hpp"

#include "ast/Program.hpp"

using namespace eddic;

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

            //The entry basic block
            function->newBasicBlock(); 

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

        }

        void operator()(ast::Assignment& assignment){

        }
        
        void operator()(ast::ArrayAssignment& assignment){

        }

        void operator()(ast::VariableDeclaration& declaration){

        }

        void operator()(ast::Swap& swap){
            auto lhs_var = swap.Content->lhs_var;
            auto rhs_var = swap.Content->rhs_var;

            //We have the guarantee here that both variables are of the same type
            switch (lhs_var->type().base()) {
                case BaseType::INT:{
                    auto temp = swap.Content->context->newTemporary();

                    function->currentBasicBlock()->add(tac::Quadruple(temp, rhs_var));  
                    function->currentBasicBlock()->add(tac::Quadruple(rhs_var, lhs_var));  
                    function->currentBasicBlock()->add(tac::Quadruple(lhs_var, temp));  

                    break;
                }
                case BaseType::STRING:{
  /*                  auto registerA = program.registers(EAX);
                   
                    auto left = lhs_var->toStringOperand();
                    auto right = rhs_var->toStringOperand();
                    
                    program.addInstruction(program.factory().createMove(left.first, registerA));
                    program.addInstruction(program.factory().createMove(right.first, left.first));
                    program.addInstruction(program.factory().createMove(registerA, right.first));
                    
                    program.addInstruction(program.factory().createMove(left.second, registerA));
                    program.addInstruction(program.factory().createMove(right.second, left.second));
                    program.addInstruction(program.factory().createMove(registerA, right.second));
    */                
                    break;
                }
                default:
                   throw SemanticalException("Variable of invalid type");
            }
        }

        void operator()(ast::While& while_){

        }

        void operator()(ast::For for_){

        }

        void operator()(ast::Foreach&){
            assert(false); //This node has been transformed into a for node
        }
       
        void operator()(ast::ForeachIn& foreach){

        }

        void operator()(ast::FunctionCall& functionCall){

        }

        void operator()(ast::Return& return_){

        }
};

void tac::TacCompiler::compile(ast::Program& program, StringPool& pool, tac::Program& tacProgram) const {
    CompilerVisitor visitor(pool, tacProgram);
    visitor(program);
}
