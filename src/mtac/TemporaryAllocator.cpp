//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <unordered_map>

#include <boost/variant.hpp>

#include "mtac/TemporaryAllocator.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"

#include "FunctionContext.hpp"

using namespace eddic;

namespace {

typedef std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<mtac::BasicBlock>> Usage;

void updateTemporary(Usage& usage, std::shared_ptr<Variable> variable, std::shared_ptr<mtac::BasicBlock> block, std::shared_ptr<mtac::Function> function){
    if(variable->position().isTemporary()){
        if(usage.find(variable) == usage.end()){
            usage[variable] = block;
        } else if(usage[variable] != block){
            function->context->storeTemporary(variable); 
        }
    }
}

template<typename T>
void updateIf(Usage& usage, std::shared_ptr<T> if_, std::shared_ptr<mtac::BasicBlock> block, std::shared_ptr<mtac::Function> function){
    if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&if_->arg1)){
        updateTemporary(usage, *variablePtr, block, function);
    }

    if(if_->arg2){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*if_->arg2)){
            updateTemporary(usage, *variablePtr, block, function);
        }
    }
}

void updateQuadruple(Usage& usage, std::shared_ptr<mtac::Quadruple> quadruple, std::shared_ptr<mtac::BasicBlock> block, std::shared_ptr<mtac::Function> function){
    if(quadruple->result){
        updateTemporary(usage, quadruple->result, block, function);
    }

    if(quadruple->arg1){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
            updateTemporary(usage, *variablePtr, block, function);
        }
    }

    if(quadruple->arg2){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
            updateTemporary(usage, *variablePtr, block, function);
        }
    }
}

}

void mtac::TemporaryAllocator::allocate(std::shared_ptr<mtac::Program> program) const {
    for(auto& function : program->functions){
        std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<BasicBlock>> usage;

        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                    updateQuadruple(usage, *ptr, block, function);
                } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
                    updateIf(usage, *ptr, block, function);
                } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
                    updateIf(usage, *ptr, block, function);
                } 
            }
        }
    }
}
