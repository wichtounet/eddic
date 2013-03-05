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
#include "mtac/ControlFlowGraph.hpp"

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

                            auto exit_copy = mtac::clone(function, exit);

                            exit->successors.erase(std::remove(exit->successors.begin(), exit->successors.end(), entry), exit->successors.end());
                            exit->successors.push_back(loop_2_entry);
                            
                            exit_copy->successors.erase(std::remove(exit_copy->successors.begin(), exit_copy->successors.end(), entry), exit_copy->successors.end());
                            exit_copy->successors.push_back(loop_1_entry);

                            mtac::BBClones bb_clones;
                            
                            bb_clones[entry] = loop_1_entry;
                            mtac::replace_bbs(bb_clones, exit);
                            
                            bb_clones[entry] = loop_2_entry;
                            mtac::replace_bbs(bb_clones, exit_copy);

                            auto after_exit = exit->next;

                            mtac::Quadruple goto_(mtac::Operator::GOTO);
                            goto_.block = after_exit;
                            
                            auto new_goto_bb = function.new_bb();
                            new_goto_bb->statements.push_back(std::move(goto_));

                            function.insert_after(function.at(exit), exit_copy);
                            function.insert_after(function.at(exit), new_goto_bb);

                            mtac::make_edge(new_goto_bb, after_exit);
                    
                            LOG<Trace>("loops") << "Unswitch loop" << log::endl;
                            function.context->global()->stats().inc_counter("loop_unswitched");

                            optimized = true;

                            std::cout << "After unswitching" << std::endl;

                            std::cout << "after_exit:" << after_exit << std::endl;
                            std::cout << "entry:" << entry << std::endl;
                            std::cout << "exit:" << exit << std::endl;
                            std::cout << "exit_copy:" << exit_copy << std::endl;
                            std::cout << "new_goto_bb:" << new_goto_bb << std::endl;
                            std::cout << "loop_1_entry:" << loop_1_entry << std::endl;
                            std::cout << "loop_2_entry:" << loop_2_entry << std::endl;

                            std::cout << function << std::endl;
                        }
                    }
                }
            }
        }
    }

    return optimized;
}
