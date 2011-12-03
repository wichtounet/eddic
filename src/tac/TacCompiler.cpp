//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "tac/TacCompiler.hpp"
#include "tac/Program.hpp"

#include "ast/Program.hpp"

using namespace eddic;

class CompilerVisitor : public boost::static_visitor<> {
    private:
        StringPool& pool;
        tac::Program& program;
    
    public:
        CompilerVisitor(StringPool& p, tac::Program& tacProgram) : pool(p), program(tacProgram) {}
        
        void operator()(ast::Program& p){

        }

        void operator()(ast::FunctionDeclaration& function){

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
