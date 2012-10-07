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
#include "Platform.hpp"
#include "Options.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"
#include "logging.hpp"

#include "mtac/TemporaryAllocator.hpp"
#include "mtac/Program.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Printer.hpp"

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

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
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

    void operator()(std::shared_ptr<mtac::If> if_){
        updateIf(if_);
    }

    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        updateIf(if_false);
    }

    void operator()(std::shared_ptr<mtac::Call> call_){
        updateTemporary(call_->return_);
        updateTemporary(call_->return2_);
    }

    void operator()(std::shared_ptr<mtac::Param> param){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param->arg)){
            updateTemporary(*ptr);
        }
    }

    template<typename T>
    void operator()(T&){
        //Ignore
    }
};

template<typename Container>
unsigned int count_temporaries(Container& container){
    unsigned int count = 0;
    
    for(auto v : container){
        if(v && v->position().isTemporary()){
            ++count;
        }
    }

    return count;
}

}

void mtac::allocate_temporary::set_platform(Platform platform){
    this->platform = platform;
}

bool mtac::allocate_temporary::operator()(std::shared_ptr<mtac::Program> program){
    for(auto& function : program->functions){
        auto count = 0;

        CollectTemporary visitor(function);

        for(auto& block : function->getBasicBlocks()){
            visitor.block = block;

            for(auto& statement : block->statements){
                visit(visitor, statement);
            }
        }
        
        mtac::LiveVariableAnalysisProblem problem;
        auto results = mtac::data_flow(function, problem);

        auto descriptor = getPlatformDescriptor(platform);
        auto registers = descriptor->number_of_registers();
        
        for(auto& block : function->getBasicBlocks()){
            for(auto& statement : block->statements){
                auto values = results->OUT_S[statement].values();
                auto temporaries = count_temporaries(values);
                
                if(temporaries >= registers){
                    auto it = values.begin();
                    auto end = values.end();

                    for(unsigned int i = 0; i <= (registers - temporaries + 1) && it != end;){
                        if((*it) && (*it)->type()->is_pointer() && (*it)->position().isTemporary()){
                            ++count;
                            function->context->storeTemporary(*it);
                            ++i;
                        }

                        ++it;
                    }
                }
            }
        }

        if(count > 0 && log::enabled<Dev>()){
            log::emit<Dev>("Optimizer") << "Temporaries have been stored for registers" << log::endl;

            mtac::print(function);
        }
    }

    //Do not restart other passes
    return false;
}
