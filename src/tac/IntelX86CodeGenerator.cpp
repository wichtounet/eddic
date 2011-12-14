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

enum Register {
    EAX,
    EBX,
    ECX,
    EDX,

    ESP, //Extended stack pointer
    EBP, //Extended base pointer

    ESI, //Extended source index
    EDI, //Extended destination index
    
    REGISTER_COUNT  
};

std::string arg(tac::Argument argument){
    //TODO

    return "";
}

struct StatementCompiler : public boost::static_visitor<> {
    AssemblyFileWriter& writer;
    std::unordered_map<std::shared_ptr<BasicBlock>, std::string> labels;
    std::unordered_map<Register, std::shared_ptr<Variable>> descriptors;

    StatementCompiler(AssemblyFileWriter& w) : writer(w) {}

    void operator()(std::shared_ptr<tac::Goto>& goto_){
       writer.stream() << "goto " << labels[goto_->block] << std::endl; 
    }

    void operator()(std::shared_ptr<tac::Param>& param){
        writer.stream() << "pushl " << arg(param->arg) << std::endl;
    }

    void operator()(std::shared_ptr<tac::Call>& call){
        writer.stream() << "call " << call->function << std::endl;
        
        if(call->params > 0){
            writer.stream() << "addl " << call->params << ", %esp" << std::endl;
        }

        //TODO Manage return values
    }
    
    void operator()(std::shared_ptr<tac::Return>& return_){
        //TODO
    }
    
    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        //TODO
    }
    
    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
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

bool updateLiveness(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, tac::Argument arg){
    if(auto* variable = boost::get<std::shared_ptr<Variable>>(&arg)){
        if(liveness.find(*variable) != liveness.end()){
            if((*variable)->position().isGlobal()){
                liveness[*variable] = true;
            } else {
                liveness[*variable] = false;
            }
        }

        bool live = liveness[*variable];

        //variable is live
        liveness[*variable] = true;

        return live;
    }

    return false;
}

void tac::IntelX86CodeGenerator::computeLiveness(std::shared_ptr<tac::Function> function){
    std::vector<std::shared_ptr<BasicBlock>>::reverse_iterator bit = function->getBasicBlocks().rbegin();
    std::vector<std::shared_ptr<BasicBlock>>::reverse_iterator bend = function->getBasicBlocks().rend(); 
    
    std::unordered_map<std::shared_ptr<Variable>, bool> liveness;

    while(bit != bend){
        std::vector<tac::Statement>::reverse_iterator sit = (*bit)->statements.rbegin();
        std::vector<tac::Statement>::reverse_iterator send = (*bit)->statements.rend(); 

        while(sit != send){
            auto statement = *sit;

            if(auto* ptr = boost::get<std::shared_ptr<tac::Param>>(&statement)){
                (*ptr)->liveVariable = updateLiveness(liveness, (*ptr)->arg);
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::Return>>(&statement)){
                if((*ptr)->arg1){
                    (*ptr)->liveVariable1 = updateLiveness(liveness, (*(*ptr)->arg1));
                }
                
                if((*ptr)->arg2){
                    (*ptr)->liveVariable2 = updateLiveness(liveness, (*(*ptr)->arg2));
                }
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                (*ptr)->liveVariable1 = updateLiveness(liveness, (*ptr)->arg1);
                (*ptr)->liveVariable2 = updateLiveness(liveness, (*ptr)->arg2);
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&statement)){
                (*ptr)->liveVariable1 = updateLiveness(liveness, (*ptr)->arg1);
                
                if((*ptr)->arg2){
                    (*ptr)->liveVariable2 = updateLiveness(liveness, (*(*ptr)->arg2));
                }

                liveness[(*ptr)->result] = false; 
            }

            sit++;
        }

        bit++;
    }
}

void tac::IntelX86CodeGenerator::compile(std::shared_ptr<tac::Function> function){
    computeLiveness(function);

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
