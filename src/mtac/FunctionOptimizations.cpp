//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>

#include "iterators.hpp"
#include "logging.hpp"
#include "GlobalContext.hpp"

#include "mtac/FunctionOptimizations.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Statement.hpp"

using namespace eddic;

namespace {

void remove_references(std::shared_ptr<mtac::Program> program, std::shared_ptr<mtac::Function> function){
    for(auto& bb : function){
        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                program->context->removeReference((*ptr)->function); 
            }
        }
    }
}

} //end of anonymous namespace

bool mtac::remove_unused_functions::operator()(std::shared_ptr<mtac::Program> program){
    auto it = iterate(program->functions);

    while(it.has_next()){
        auto function = *it;

        if(program->context->referenceCount(function->getName()) == 0){
            remove_references(program, function);
            log::emit<Debug>("Optimizer") << "Remove unused function " << function->getName() << log::endl;
            it.erase();
            continue;
        } else if(program->context->referenceCount(function->getName()) == 1 && mtac::is_recursive(function)){
            remove_references(program, function);
            log::emit<Debug>("Optimizer") << "Remove unused recursive function " << function->getName() << log::endl;
            it.erase();
            continue;
        } 

        ++it;
    }

    //Not necessary to restart the other passes
    return false;
}

bool mtac::remove_empty_functions::operator()(std::shared_ptr<mtac::Program> program){
    std::vector<std::string> removed_functions;

    auto it = iterate(program->functions);

    while(it.has_next()){
        auto function = *it;

        if(function->getName() == "_F4main" || function->getName() == "_F4mainAS"){
            ++it;
            continue;
        }

        unsigned int statements = function->size();

        if(statements == 0){
            removed_functions.push_back(function->getName());
            it.erase();
        } else {
            ++it;
        }
    }

    if(!removed_functions.empty()){
        for(auto& function : program->functions){
            for(auto& block : function){
                auto fit = block->statements.begin();

                while(fit != block->statements.end()){
                    auto statement = *fit;

                    if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                        auto function = (*ptr)->function;

                        if(std::find(removed_functions.begin(), removed_functions.end(), function) != removed_functions.end()){
                            int parameters = (*ptr)->functionDefinition->parameters.size();

                            if(parameters > 0){
                                //The parameters are in the previous block
                                if(fit == block->statements.begin()){
                                    auto previous = block->prev;

                                    auto fend = previous->statements.end();
                                    --fend;

                                    while(parameters > 0){
                                        fend = previous->statements.erase(fend);
                                        --fend;

                                        --parameters;
                                    }

                                    fit = block->statements.erase(fit);
                                } 
                                //The parameters are in the same block
                                else {
                                    while(parameters >= 0){
                                        fit = block->statements.erase(fit);
                                        --fit;

                                        --parameters;
                                    }
                                }

                            } else {
                                fit = block->statements.erase(fit);
                            }

                            continue;
                        }
                    }

                    ++fit;
                }
            }
        }
    }

    //Not necessary to restart the other passes
    return false;
}
