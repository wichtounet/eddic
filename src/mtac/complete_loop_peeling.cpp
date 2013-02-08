//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/range/adaptors.hpp>

#include "iterators.hpp"
#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "logging.hpp"

#include "mtac/loop.hpp"
#include "mtac/complete_loop_peeling.hpp"
#include "mtac/Function.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Program.hpp"

using namespace eddic;

namespace {

void remove_back_edge(mtac::loop& loop){
    mtac::remove_edge(loop.find_exit(), loop.find_entry());
}

}

bool mtac::complete_loop_peeling::gate(std::shared_ptr<Configuration> configuration){
    return configuration->option_defined("fcomplete-peel-loops");
}

bool mtac::complete_loop_peeling::operator()(mtac::Function& function){
    if(function.loops().empty()){
        return false;
    }

    bool optimized = false;
    
    auto lit = iterate(function.loops());

    while(lit.has_next()){
        auto loop = *lit;

        if(loop.has_estimate() && loop.single_exit()){
            auto iterations = loop.estimate();

            if(iterations > 0 && iterations < 12){
                optimized = true;

                LOG<Trace>("loops") << "Completely peel " << loop << " with "  << iterations << " iterations" << log::endl;
                function.context->global()->stats().inc_counter("loop_peeled");

                //The comparison is not necessary anymore
                auto exit = loop.find_exit();
                exit->statements.pop_back();

                auto real_entry = loop.find_entry();
                auto entry = real_entry;

                mtac::basic_block_p next_bb;
                for(auto& succ : exit->successors){
                    if(loop.blocks().find(succ) == loop.blocks().end()){
                        next_bb = succ;
                    }
                }

                //Remove the back edge
                remove_back_edge(loop);
                
                //There are perhaps new references to functions
                for(auto& bb : loop){
                    for(auto& statement : bb->statements){
                        if(statement.op == mtac::Operator::CALL){
                            program.call_graph.edge(function.definition(), statement.function())->count += (iterations - 1);
                        }
                    }
                }
                
                auto preheader = mtac::find_safe_preheader(loop, function, true);
                auto it = function.at(preheader);
                    
                std::vector<mtac::basic_block_p> source_bbs;

                for(auto& bb : loop){
                    source_bbs.push_back(bb);
                }

                std::sort(source_bbs.begin(), source_bbs.end(), 
                        [&function](const mtac::basic_block_p& lhs, const mtac::basic_block_p& rhs){ return function.position(lhs) > function.position(rhs);});

                std::vector<mtac::basic_block_p> cloned;

                for(int i = 1; i < iterations; ++i){
                    std::vector<mtac::basic_block_p> local_cloned;
                    std::unordered_map<mtac::basic_block_p, mtac::basic_block_p> bb_clones;

                    //Copy all the basic blocks

                    for(auto& bb : source_bbs){
                        auto clone_bb = mtac::clone(function, bb);
                
                        LOG<Trace>("loops") << "Cloned " << bb << " into " << clone_bb << log::endl;

                        bb_clones[bb] = clone_bb;
                        local_cloned.push_back(clone_bb);
                        cloned.push_back(clone_bb);

                        function.insert_after(it, clone_bb);
                    }

                    //Adapt the CFG
                    
                    for(auto& clone : local_cloned){
                        for(auto& succ : clone->successors){
                            if(bb_clones.find(succ) != bb_clones.end()){
                                succ = bb_clones[succ];
                            } else if(succ == next_bb){
                                succ = entry;
                            }
                        }
                        
                        for(auto& pred : clone->predecessors){
                            if(bb_clones.find(pred) != bb_clones.end()){
                                pred = bb_clones[pred];
                            }
                        }
                        
                        //Adapt the instructions themselves
                        for(auto& goto_ : clone){
                            if(bb_clones.find(goto_.block) != bb_clones.end()){
                                goto_.block = bb_clones[goto_.block];
                            }
                        }
                    }

                    if(entry != real_entry){
                        for(auto& pred : entry->predecessors){
                            if(pred == preheader){
                                pred = bb_clones[exit];
                            }
                        }
                    }

                    entry = bb_clones[real_entry];

                    if(i == iterations -1){
                        preheader->successors.front() = entry;
                    }
                }
                
                for(auto& pred : real_entry->predecessors){
                    if(pred == preheader){
                        pred = real_entry->prev;
                    }
                }

                //It is a not loop anymore
                lit.erase();

                continue;
            }
        }

        ++lit;
    }

    return optimized;
}
