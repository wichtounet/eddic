//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "PerfsTimer.hpp"
#include "logging.hpp"

#include "mtac/GlobalOptimizations.hpp"

#include "ltac/register_allocator.hpp"
#include "ltac/LiveRegistersProblem.hpp"
#include "ltac/interference_graph.hpp"

using namespace eddic;

namespace {

void renumber(mtac::function_p function){
    //TODO
}

template<typename Opt>
void gather_reg(Opt& reg, ltac::interference_graph& graph){
    if(reg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*reg)){
            graph.gather(*ptr);
        }
    }
}

template<typename Opt>
void gather(Opt& arg, ltac::interference_graph& graph){
    if(arg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*arg)){
            graph.gather(*ptr);
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            gather_reg(ptr->base_register, graph);
            gather_reg(ptr->scaled_register, graph);
        }
    }
}

void gather_pseudo_regs(mtac::function_p function, ltac::interference_graph& graph){
    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                gather((*ptr)->arg1, graph);
                gather((*ptr)->arg2, graph);
                gather((*ptr)->arg3, graph);
            }
        }
    }

    log::emit<Trace>("registers") << "Found " << graph.size() << " pseudo registers" << log::endl;
}

void build_interference_graph(ltac::interference_graph& graph, mtac::function_p function){
    //Init the graph structure with the current size
    gather_pseudo_regs(function, graph);
    graph.build_graph();

    ltac::LiveRegistersProblem problem;
    auto live_results = mtac::data_flow(function, problem);

    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            auto& live_registers = live_results->OUT_LS[statement].values().registers;

            if(live_registers.size() > 1){
                auto it = live_registers.begin();
                auto end = live_registers.end();

                while(it != end){
                    auto next = it;
                    ++next;

                    while(next != end){
                        graph.add_edge(graph.convert(*it), graph.convert(*next));

                        ++next;
                    }

                    ++it;
                }
            }
        }
    }

    graph.build_adjacency_vectors();
}

static const std::size_t store_cost = 5;
static const std::size_t load_cost = 3;

std::size_t depth_cost(unsigned int depth){
    unsigned int cost = 1;

    while(depth > 0){
        cost *= 10;

        --depth;
    }

    return cost;
}

template<typename Opt>
void update_cost_reg(Opt& reg, ltac::interference_graph& graph, unsigned int depth){
    if(reg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*reg)){
            graph.spill_cost(graph.convert(*ptr)) += load_cost * depth_cost(depth);
        }
    }
}

template<typename Opt>
void update_cost(Opt& arg, ltac::interference_graph& graph, unsigned int depth){
    if(arg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*arg)){
            graph.spill_cost(graph.convert(*ptr)) += load_cost * depth_cost(depth);
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            update_cost_reg(ptr->base_register, graph, depth);
            update_cost_reg(ptr->scaled_register, graph, depth);
        }
    }
}

void estimate_spill_costs(mtac::function_p function, ltac::interference_graph& graph){
    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                if(ltac::erase_result((*ptr)->op)){
                    if(auto* reg_ptr = boost::get<ltac::PseudoRegister>(&*(*ptr)->arg1)){
                        graph.spill_cost(graph.convert(*reg_ptr)) += store_cost * depth_cost(bb->depth);
                    }
                } else {
                    update_cost((*ptr)->arg1, graph, bb->depth);
                }

                update_cost((*ptr)->arg2, graph, bb->depth);
                update_cost((*ptr)->arg3, graph, bb->depth);
            }
        }
    }
}

void simplify(ltac::interference_graph& graph, Platform platform, std::vector<std::size_t>& spilled, std::list<std::size_t>& order){
    std::set<std::size_t> n;
    for(std::size_t r = 0; r < graph.size(); ++r){
        n.insert(r);
    }

    auto descriptor = getPlatformDescriptor(platform);
    auto K = descriptor->number_of_registers();

    log::emit<Trace>("registers") << "Attempt a " << K << "-coloring of the graph" << log::endl;

    while(!n.empty()){
        std::size_t node;
        bool found = false;

        for(auto& candidate : n){
            if(graph.degree(candidate) < K){
                node = candidate;        
                found = true;
                break;
            }
        }
        
        if(!found){
            //TODO Spills the node with the minimal cost
        }

        n.erase(node);
        graph.remove_node(node);

        order.push_back(node);
    }
}

void spill_code(ltac::interference_graph& graph, mtac::function_p function, std::vector<std::size_t>& spilled){
    //TODO
}

template<typename Opt>
void update_reg(Opt& reg, std::unordered_map<ltac::PseudoRegister, ltac::Register>& register_allocation){
    if(reg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*reg)){
            reg = register_allocation[*ptr];
        }
    }
}

template<typename Opt>
void update(Opt& arg, std::unordered_map<ltac::PseudoRegister, ltac::Register>& register_allocation){
    if(arg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*arg)){
            arg = register_allocation[*ptr];
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            update_reg(ptr->base_register, register_allocation);
            update_reg(ptr->scaled_register, register_allocation);
        }
    }
}

void replace_registers(mtac::function_p function, std::unordered_map<std::size_t, std::size_t>& allocation, ltac::interference_graph& graph){
    std::unordered_map<ltac::PseudoRegister, ltac::Register> register_allocation;

    for(auto& pair : allocation){
        register_allocation[graph.convert(pair.first)] = {pair.second};
    }

    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                update((*ptr)->arg1, register_allocation);
                update((*ptr)->arg2, register_allocation);
                update((*ptr)->arg3, register_allocation);
            }
        }
    }
}

void select(ltac::interference_graph& graph, mtac::function_p function, Platform platform, std::list<std::size_t>& order){
    std::unordered_map<std::size_t, std::size_t> allocation;
    
    auto descriptor = getPlatformDescriptor(platform);
    auto colors = descriptor->symbolic_registers();

    //Handle bound registers
    auto it = iterate(order);
    while(it.has_next()){
        auto reg = *it;

        if(graph.convert(reg).bound){
            log::emit<Trace>("registers") << "Alloc " << graph.convert(reg).binding << " to pseudo " << reg << " (bound)" << log::endl;
            allocation[reg] = graph.convert(reg).binding;
            it.erase();
        } else {
            ++it;
        }
    }

    while(!order.empty()){
        std::size_t reg = order.back();
        order.pop_back();

        for(auto color : colors){
            bool found = false;

            for(auto neighbor : graph.neighbors(reg)){
                if(allocation.count(neighbor)){
                    if(allocation[neighbor] == color){
                        found = true;
                        break;
                    }
                }
            }

            if(!found){
                log::emit<Trace>("registers") << "Alloc " << color << " to pseudo " << reg << log::endl;
                allocation[reg] = color;
                break;
            }
        }
    }

    replace_registers(function, allocation, graph);
}

void register_allocation(mtac::function_p function, Platform platform){
    log::emit<Trace>("registers") << "Allocate registers for function " << function->getName() << log::endl;

    while(true){
        //1. Renumber
        renumber(function);

        //2. Build
        ltac::interference_graph graph;
        build_interference_graph(graph, function);

        //3. Coalesce
        //TODO

        //4. Spill costs
        estimate_spill_costs(function, graph);

        //5. Simplify
        std::vector<std::size_t> spilled;
        std::list<std::size_t> order;
        simplify(graph, platform, spilled, order);

        if(!spilled.empty()){
            //6. Spill code
            spill_code(graph, function, spilled);
        } else {
            //7. Select
            select(graph, function, platform, order);

            return;
        }
    }
}

} //end of anonymous namespace

void ltac::register_allocation(std::shared_ptr<mtac::Program> program, Platform platform){
    PerfsTimer timer("Register allocation");

    for(auto& function : program->functions){
        ::register_allocation(function, platform);
    }
}
