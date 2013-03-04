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
#include "mtac/variable_usage.hpp"

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
                            auto usage = mtac::compute_write_usage(loop);

                            if(condition.arg1){
                                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*condition.arg1)){
                                    if(usage.written[*ptr] > 0){
                                        continue;
                                    }
                                }
                            }
                            
                            if(condition.arg2){
                                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*condition.arg2)){
                                    if(usage.written[*ptr] > 0){
                                        continue;
                                    }
                                }
                            }

                            auto loop_1_entry = entry->successors.front();
                            auto loop_2_entry = entry->successors.back();
                            
                            entry->predecessors.erase(std::remove(entry->predecessors.begin(), entry->predecessors.end(), entry), entry->predecessors.end());

                            exit_copy = mtac::clone(function, exit);
                            
                            exit_copy->successors.erase(std::remove(exit_copy->successors.begin(), exit_copy->successors.end(), entry), exit_copy->successors.end());
                            exit_copy->successors.push_back(loop_1_entry);

                            exit->successors.erase(std::remove(exit->successors.begin(), exit->successors.end(), entry), exit->successors.end());
                            exit->successors.push_back(loop_2_entry);

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
