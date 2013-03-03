//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "logging.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"

#include "mtac/Function.hpp"
#include "mtac/loop.hpp"
#include "mtac/loop_unswitching.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

bool mtac::loop_unswitching::gate(std::shared_ptr<Configuration> configuration){
    return configuration->option_defined("funswitch-loops");
}

bool mtac::loop_unswitching::operator()(mtac::Function& function){
    if(function.loops().empty()){
        return false;
    }

    bool optimized = false;

    for(auto& loop : function.loops()){
        if(loop.single_exit()){
            auto entry = loop.find_entry();
            auto exit = loop.find_exit();

            if(entry && exit && entry->size() == 1){
                if(entry->successors.size() == 2 && exit->predecessors.size() == 2){
                    if(std::is_permutation(entry->successors.begin(), entry->successors.end(), exit->predecessors.begin())){
                        auto& condition = *entry->begin();

                        if(condition.is_if() || condition.is_if_false()){
                            //TODO Test if the condition is invariant
                            //TODO Unswitch the loop
                            
                            std::cout << "Find out" << std::endl;
                        }
                    }
                }
            }
        }
    }

    return optimized;
}
