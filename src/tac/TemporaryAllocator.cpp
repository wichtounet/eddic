//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <unordered_map>

#include <boost/variant.hpp>

#include "tac/TemporaryAllocator.hpp"
#include "tac/Program.hpp"
#include "tac/Utils.hpp"

#include "FunctionContext.hpp"

using namespace eddic;

namespace {

typedef std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<tac::BasicBlock>> Usage;

void updateTemporary(Usage usage, std::shared_ptr<Variable> variable, std::shared_ptr<tac::BasicBlock> block, std::shared_ptr<tac::Function> function){
    if(variable->position().isTemporary()){
        if(usage.find(variable) == usage.end()){
            usage[variable] = block;
        } else if(usage[variable] != block){
            function->context->storeTemporary(variable); 
        }
    }
}

template<typename T>
void updateIf(Usage usage, std::shared_ptr<T> if_, std::shared_ptr<tac::BasicBlock> block, std::shared_ptr<tac::Function> function){
    if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&if_->arg1)){
        updateTemporary(usage, *variablePtr, block, function);
    }

    if(if_->arg2){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*if_->arg2)){
            updateTemporary(usage, *variablePtr, block, function);
        }
    }
}

void updateQuadruple(Usage usage, std::shared_ptr<tac::Quadruple> quadruple, std::shared_ptr<tac::BasicBlock> block, std::shared_ptr<tac::Function> function){
    updateTemporary(usage, quadruple->result, block, function);

    updateIf(usage, quadruple, block, function);
}

}

void tac::TemporaryAllocator::allocate(tac::Program& program) const {
    for(auto& function : program.functions){
        std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<BasicBlock>> usage;

        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&statement)){
                    updateQuadruple(usage, *ptr, block, function);
                } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                    updateIf(usage, *ptr, block, function);
                } else if(auto* ptr = boost::get<std::shared_ptr<tac::If>>(&statement)){
                    updateIf(usage, *ptr, block, function);
                } else if(auto* ptr = boost::get<std::shared_ptr<tac::Return>>(&statement)){
                    if((*ptr)->arg1){
                        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*(*ptr)->arg1)){
                            updateTemporary(usage, *variablePtr, block, function);
                        }
                    }
                }
            }
        }
    }
}
