//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <unordered_map>

#include "variant.hpp"
#include "FunctionContext.hpp"

#include "mtac/TemporaryAllocator.hpp"
#include "mtac/Program.hpp"
#include "mtac/Utils.hpp"

#include "VisitorUtils.hpp"

using namespace eddic;

namespace {

typedef std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<mtac::BasicBlock>> Usage;

struct CollectTemporary : public boost::static_visitor<> {
    std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<mtac::BasicBlock>> usage;
    std::shared_ptr<mtac::BasicBlock> block;
    std::shared_ptr<mtac::Function> function;

    CollectTemporary(std::shared_ptr<mtac::Function> function) : function(function) {}

    void updateTemporary(std::shared_ptr<Variable> variable){
        if(variable && variable->position().isTemporary()){
            if(usage.find(variable) == usage.end()){
                usage[variable] = block;
            } else if(usage[variable] != block){
                function->context->storeTemporary(variable); 
            }
        }
    }

    void operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
        if(quadruple->result){
            updateTemporary(quadruple->result);
        }

        if(quadruple->arg1){
            if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                updateTemporary(*variablePtr);
            }
        }

        if(quadruple->arg2){
            if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                updateTemporary(*variablePtr);
            }
        }
    }

    template<typename T>
    void updateIf(std::shared_ptr<T> if_){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&if_->arg1)){
            updateTemporary(*variablePtr);
        }

        if(if_->arg2){
            if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*if_->arg2)){
                updateTemporary(*variablePtr);
            }
        }
    }

    void operator()(std::shared_ptr<mtac::If>& if_){
        updateIf(if_);
    }

    void operator()(std::shared_ptr<mtac::IfFalse>& if_false){
        updateIf(if_false);
    }

    void operator()(std::shared_ptr<mtac::Call>& call_){
        updateTemporary(call_->return_);
        updateTemporary(call_->return2_);
    }

    void operator()(std::shared_ptr<mtac::Param>& param){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param->arg)){
            updateTemporary(*ptr);
        }
    }

    template<typename T>
    void operator()(T&){
        //Ignore
    }
};

}

void mtac::allocate_temporary(std::shared_ptr<mtac::Program> program){
    for(auto& function : program->functions){
        CollectTemporary visitor(function);

        for(auto& block : function->getBasicBlocks()){
            visitor.block = block;

            for(auto& statement : block->statements){
                visit(visitor, statement);
            }
        }
    }
}
