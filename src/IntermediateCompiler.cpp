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

#include "il/IntermediateProgram.hpp"

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

            //visit_each(*this, function.instructions);

            program.addInstruction(program.factory().createFunctionExit(function.context->size()));
        }

        void operator()(GlobalVariableDeclaration& variable){
            //Nothing to compile, the global variable values are written using global contexts
        }
};

void IntermediateCompiler::compile(ASTProgram& program, StringPool& pool, IntermediateProgram& intermediateProgram){
    CompilerVisitor visitor(pool, intermediateProgram);
    visitor(program);
}
