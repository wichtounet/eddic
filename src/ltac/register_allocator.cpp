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

//TODO Store the index in the pseudo for ease
typedef std::vector<ltac::PseudoRegister> pseudo_reg; //Maps indices to pseudo regs
typedef std::unordered_map<ltac::PseudoRegister, std::size_t> reg_pseudo; //Maps pseudo regs to indices

void renumber(mtac::function_p function){
    //TODO
}

template<typename Opt>
void gather_reg(Opt& reg, pseudo_reg& pseudo_registers, reg_pseudo& current){
    if(reg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*reg)){
            if(!current.count(*ptr)){
                current[*ptr] = pseudo_registers.size();
                pseudo_registers.push_back(*ptr);
            }
        }
    }
}

template<typename Opt>
void gather(Opt& arg, pseudo_reg& pseudo_registers, reg_pseudo& current){
    if(arg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*arg)){
            if(!current.count(*ptr)){
                current[*ptr] = pseudo_registers.size();
                pseudo_registers.push_back(*ptr);
            }
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            gather_reg(ptr->base_register, pseudo_registers, current);
            gather_reg(ptr->scaled_register, pseudo_registers, current);
        }
    }
}

void gather_pseudo_regs(mtac::function_p function, pseudo_reg& pseudo_registers, reg_pseudo& current){
    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                gather((*ptr)->arg1, pseudo_registers, current);
                gather((*ptr)->arg2, pseudo_registers, current);
                gather((*ptr)->arg3, pseudo_registers, current);
            }
        }
    }

    log::emit<Trace>("registers") << "Found " << pseudo_registers.size() << " pseudo registers" << log::endl;
}

void build_interference_graph(ltac::interference_graph& graph, mtac::function_p function, reg_pseudo& registers_to_index){
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
                        graph.add_edge(registers_to_index[*it], registers_to_index[*next]);

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
void update_cost_reg(Opt& reg, std::unordered_map<ltac::PseudoRegister, std::size_t>& spill_costs, unsigned int depth){
    if(reg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*reg)){
            spill_costs[*ptr] += load_cost * depth_cost(depth);
        }
    }
}

template<typename Opt>
void update_cost(Opt& arg, std::unordered_map<ltac::PseudoRegister, std::size_t>& spill_costs, unsigned int depth){
    if(arg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&*arg)){
            spill_costs[*ptr] += load_cost * depth_cost(depth);
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            update_cost_reg(ptr->base_register, spill_costs, depth);
            update_cost_reg(ptr->scaled_register, spill_costs, depth);
        }
    }
}

void estimate_spill_costs(mtac::function_p function, std::unordered_map<ltac::PseudoRegister, std::size_t>& spill_costs){
    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                if(ltac::erase_result((*ptr)->op)){
                    if(auto* reg_ptr = boost::get<ltac::PseudoRegister>(&*(*ptr)->arg1)){
                        spill_costs[*reg_ptr] += store_cost * depth_cost(bb->depth);
                    }
                } else {
                    update_cost((*ptr)->arg1, spill_costs, bb->depth);
                }

                update_cost((*ptr)->arg2, spill_costs, bb->depth);
                update_cost((*ptr)->arg3, spill_costs, bb->depth);
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

void replace_registers(mtac::function_p function, std::unordered_map<std::size_t, std::size_t>& allocation, pseudo_reg& pseudo_registers){
    std::unordered_map<ltac::PseudoRegister, ltac::Register> register_allocation;

    for(auto& pair : allocation){
        register_allocation[pseudo_registers[pair.first]] = {pair.second};
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

void select(ltac::interference_graph& graph, mtac::function_p function, Platform platform, std::list<std::size_t>& order, pseudo_reg& pseudo_registers){
    std::unordered_map<std::size_t, std::size_t> allocation;
    
    auto descriptor = getPlatformDescriptor(platform);
    auto colors = descriptor->symbolic_registers();

    //Handle bound registers
    auto it = iterate(order);
    while(it.has_next()){
        auto reg = *it;

        if(pseudo_registers[reg].bound){
            log::emit<Trace>("registers") << "Alloc " << pseudo_registers[reg].binding << " to pseudo " << reg << " (bound)" << log::endl;
            allocation[reg] = pseudo_registers[reg].binding;
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

            std::cout << graph.neighbors(reg).size() << std::endl;

            if(!found){
                log::emit<Trace>("registers") << "Alloc " << color << " to pseudo " << reg << log::endl;
                allocation[reg] = color;
                break;
            }
        }
    }

    replace_registers(function, allocation, pseudo_registers);
}

void register_allocation(mtac::function_p function, Platform platform){
    log::emit<Trace>("registers") << "Allocate registers for function " << function->getName() << log::endl;

    while(true){
        //1. Renumber
        renumber(function);

        //2. Build
        reg_pseudo registers_to_index;
        pseudo_reg registers;
        gather_pseudo_regs(function, registers, registers_to_index);

        //Starting from there, only the internal index is used (until 6/7 pass)

        ltac::interference_graph graph(registers.size());
        build_interference_graph(graph, function, registers_to_index);

        //3. Coalesce

        //4. Spill costs
        std::unordered_map<ltac::PseudoRegister, std::size_t> spill_costs;
        estimate_spill_costs(function, spill_costs);

        //5. Simplify
        std::vector<std::size_t> spilled;
        std::list<std::size_t> order;
        simplify(graph, platform, spilled, order);

        if(!spilled.empty()){
            //6. Spill code
            spill_code(graph, function, spilled);
        } else {
            //7. Select
            select(graph, function, platform, order, registers);

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
