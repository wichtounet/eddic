//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <unordered_map>

#include <boost/variant.hpp>

#include "tac/IntelX86CodeGenerator.hpp"

#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"

#include "il/Labels.hpp"

using namespace eddic;

tac::IntelX86CodeGenerator::IntelX86CodeGenerator(AssemblyFileWriter& w) : writer(w) {}

namespace eddic { namespace tac { 

struct StatementCompiler : public boost::static_visitor<> {
    AssemblyFileWriter& writer;
    std::unordered_map<std::shared_ptr<BasicBlock>, std::string> labels;

    StatementCompiler(AssemblyFileWriter& w) : writer(w) {}

    void operator()(tac::Goto& goto_){
       writer.stream() << "goto " << labels[goto_.block] << std::endl; 
    }

    void operator()(tac::Param& param){
        //TODO
    }

    void operator()(tac::Call& call){
        writer.stream() << "call " << call.function << std::endl;
        
        if(call.params > 0){
            writer.stream() << "addl " << call.params << ", %esp" << std::endl;
        }

        //TODO Manage return values
    }
    
    void operator()(tac::Return& return_){
        //TODO
    }
    
    void operator()(tac::Quadruple& quadruple){
        //TODO
    }
    
    void operator()(tac::IfFalse& ifFalse){
        //TODO
    }

    void operator()(std::string&){
        assert(false); //There is no more label after the basic blocks have been extracted
    }
};

}}

void tac::IntelX86CodeGenerator::compile(std::shared_ptr<tac::BasicBlock> block, StatementCompiler& compiler){
    std::string label = newLabel();

    compiler.labels[block] = label;

    writer.stream() << label << ":" << std::endl;

    for(auto& statement : block->statements){
        boost::apply_visitor(compiler, statement);
    }
}

void tac::IntelX86CodeGenerator::compile(std::shared_ptr<tac::Function> function){
    writer.stream() << std::endl << function->getName() << ":" << std::endl;
    
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;

    auto size = function->context->size();
    //Only if necessary, allocates size on the stack for the local variables
    if(size > 0){
        writer.stream() << "subl $" << size << " , %esp" << std::endl;
    }

    StatementCompiler compiler(writer);
    for(auto& block : function->getBasicBlocks()){
        compile(block, compiler);
    }
    
    //Only if necessary, deallocates size on the stack for the local variables
    if(size > 0){
        writer.stream() << "addl $" << size << " , %esp" << std::endl;
    }
    
    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void tac::IntelX86CodeGenerator::generate(tac::Program& program){
    resetNumbering();

    for(auto& function : program.functions){
        compile(function);
    }
}
