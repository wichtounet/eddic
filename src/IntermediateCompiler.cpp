//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "IntermediateCompiler.hpp"

#include "StringPool.hpp"
#include "VisitorUtils.hpp"

#include "Context.hpp"

#include "il/IntermediateProgram.hpp"

#include "ast/Program.hpp"

using namespace eddic;

struct CompilerVisitor : public boost::static_visitor<> {
    void operator()(ASTProgram& p){
        //TODO main

        //visit_each(*this, p.blocks);
              //program->addFirst(std::shared_ptr<ParseNode>(new MainDeclaration(program->context(), parser.getLexer().getDefaultToken())));
              //program->addLast(std::shared_ptr<ParseNode>(new Methods(program->context(), parser.getLexer().getDefaultToken())));

        //TODO methods
        //TODO pool

        p.context->writeIL(program);
    }

    CompilerVisitor(StringPool& p, IntermediateProgram& intermediateProgram) : pool(p), program(intermediateProgram) {}

    private:
        StringPool& pool;
        IntermediateProgram& program;
};

void IntermediateCompiler::compile(ASTProgram& program, StringPool& pool, IntermediateProgram& intermediateProgram){
    CompilerVisitor visitor(pool, intermediateProgram);
    visitor(program);
}
