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

#include "tac/TacCompiler.hpp"
#include "tac/Program.hpp"

#include "ast/Program.hpp"

//TODO Move label generator system in another folder
#include "il/Labels.hpp"

using namespace eddic;

struct AssignValueToVariable : public boost::static_visitor<> {
    AssignValueToVariable(std::shared_ptr<tac::Function> f, std::shared_ptr<Variable> v) : function(f), variable(v) {}
    
    mutable std::shared_ptr<tac::Function> function;
    std::shared_ptr<Variable> variable;

    void operator()(ast::Litteral& litteral) const {
        //TODO
    }

    void operator()(ast::Integer& integer) const {
        function->add(tac::Quadruple(variable, integer.value));
    }

    void operator()(ast::FunctionCall& call) const {
        //TODO
    }

    void operator()(ast::VariableValue& value) const {
        //TODO
    }

    void operator()(ast::ArrayValue& value) const {
        //TODO
    }

    void operator()(ast::ComposedValue& value) const {
        //TODO
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
