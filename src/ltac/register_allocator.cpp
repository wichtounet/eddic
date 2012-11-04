//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <list>

#include "PerfsTimer.hpp"
#include "logging.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "Type.hpp"

#include "mtac/Statement.hpp"
#include "mtac/GlobalOptimizations.hpp"

#include "ltac/Statement.hpp"
#include "ltac/LiveRegistersProblem.hpp"
#include "ltac/register_allocator.hpp"
#include "ltac/interference_graph.hpp"
#include "ltac/Utils.hpp"
#include "ltac/Printer.hpp"

/*
 * Register allocation using Chaitin-style graph coloring allocation. 
 *
 * The renumber and coalescing are simplified by renumbering coalescing 
 * only pseudo registers that are local to a basic block. 
 *
 * TODO:
 *  - Use Chaitin-Briggs optimistic coloring
 *  - Implement rematerialization
 *  - Use UD-chains and make renumber and coalescing complete
 */

using namespace eddic;

namespace {

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoRegister>::value, std::size_t>::type last_register(mtac::function_p function){
    return function->pseudo_registers();
}

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoFloatRegister>::value, std::size_t>::type last_register(mtac::function_p function){
    return function->pseudo_float_registers();
}

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoRegister>::value, void>::type set_last_reg(mtac::function_p function, std::size_t reg){
    function->set_pseudo_registers(reg);
}

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoFloatRegister>::value, void>::type set_last_reg(mtac::function_p function, std::size_t reg){
    function->set_pseudo_float_registers(reg);
}

template<typename Source, typename Target, typename Opt>
void update_reg(Opt& reg, std::unordered_map<Source, Target>& register_allocation){
    if(reg){
        if(auto* ptr = boost::get<Source>(&*reg)){
            if(register_allocation.count(*ptr)){
                reg = register_allocation[*ptr];
            }
        }
    }
}

template<typename Source, typename Target, typename Opt>
void update(Opt& arg, std::unordered_map<Source, Target>& register_allocation){
    if(arg){
        if(auto* ptr = boost::get<Source>(&*arg)){
            if(register_allocation.count(*ptr)){
                arg = register_allocation[*ptr];
            }
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            update_reg(ptr->base_register, register_allocation);
            update_reg(ptr->scaled_register, register_allocation);
        }
    }
}

template<typename Source, typename Target>
void replace_registers(mtac::function_p function, std::unordered_map<Source, Target>& register_allocation){
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

template<typename Opt, typename Pseudo>
bool contains_reg_addr(Opt& reg, Pseudo search_reg){
    if(reg){
        if(auto* ptr = boost::get<Pseudo>(&*reg)){
            return search_reg == *ptr;
        }
    }

    return false;
}

template<typename Opt, typename Pseudo>
bool contains_reg(Opt& arg, Pseudo reg){
    if(arg){
        if(auto* ptr = boost::get<Pseudo>(&*arg)){
            return reg == *ptr;
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            return contains_reg_addr(ptr->base_register, reg)
                || contains_reg_addr(ptr->scaled_register, reg);
        }
    }

    return false;
}

//Must be called after is_store

template<typename Pseudo>
bool is_load(ltac::Statement& statement, const Pseudo& reg){
    if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        return contains_reg((*ptr)->arg1, reg) 
            || contains_reg((*ptr)->arg2, reg)
            || contains_reg((*ptr)->arg3, reg);
    }

    return false;
}

template<typename Pseudo>
bool is_store_complete(ltac::Statement& statement, const Pseudo& reg){
    if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        if(ltac::erase_result_complete((*ptr)->op)){
            if(auto* reg_ptr = boost::get<Pseudo>(&*(*ptr)->arg1)){
                return *reg_ptr == reg;
            }
        }
    }

    return false;
}

template<typename Pseudo>
bool is_store(ltac::Statement& statement, Pseudo reg){
    if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        if(ltac::erase_result((*ptr)->op)){
            if(auto* reg_ptr = boost::get<Pseudo>(&*(*ptr)->arg1)){
                return *reg_ptr == reg;
            }
        }
    }

    return false;
}

template<typename Pseudo>
void replace_reg_addr(boost::optional<ltac::AddressRegister>& reg, Pseudo source, Pseudo target){
    if(reg){
        if(auto* ptr = boost::get<Pseudo>(&*reg)){
            if(*ptr == source){
                reg = target;
            }
        }
    }
}

template<typename Pseudo>
void replace_register(boost::optional<ltac::Argument>& arg, Pseudo source, Pseudo target){
    if(arg){
        if(auto* ptr = boost::get<Pseudo>(&*arg)){
            if(*ptr == source){
                arg = target;
            }
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            replace_reg_addr(ptr->base_register, source, target);
            replace_reg_addr(ptr->scaled_register, source, target);
        }
    }
}

template<typename Pseudo>
void replace_register(ltac::Statement& statement, Pseudo source, Pseudo target){
    if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        replace_register((*ptr)->arg1, source, target);
        replace_register((*ptr)->arg2, source, target);
        replace_register((*ptr)->arg3, source, target);
    }
}

//1. Renumber

template<typename Pseudo>
using local_reg = std::unordered_map<mtac::basic_block_p, std::unordered_set<Pseudo>>;

template<typename Opt, typename Pseudo>
void find_reg_addr(Opt& reg, std::unordered_set<Pseudo>& registers){
    if(reg){
        if(auto* ptr = boost::get<Pseudo>(&*reg)){
            registers.insert(*ptr);
        }
    }
}

template<typename Opt, typename Pseudo>
void find_reg(Opt& arg, std::unordered_set<Pseudo>& registers){
    if(arg){
        if(auto* ptr = boost::get<Pseudo>(&*arg)){
            registers.insert(*ptr);
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            find_reg_addr(ptr->base_register, registers);
            find_reg_addr(ptr->scaled_register, registers);
        }
    }
}

template<typename Stmt, typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoRegister>::value, void>::type 
get_special_uses(Stmt& instruction, std::unordered_set<Pseudo>& local_pseudo_registers){
    for(auto reg : instruction->uses){
        local_pseudo_registers.insert(reg);
    }
}

template<typename Stmt, typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoFloatRegister>::value, void>::type 
get_special_uses(Stmt& instruction, std::unordered_set<Pseudo>& local_pseudo_registers){
    for(auto reg : instruction->float_uses){
        local_pseudo_registers.insert(reg);
    }
}

template<typename Pseudo>
void find_local_registers(mtac::function_p function, local_reg<Pseudo>& local_pseudo_registers){
    local_reg<Pseudo> pseudo_registers;

    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                find_reg((*ptr)->arg1, pseudo_registers[bb]);
                find_reg((*ptr)->arg2, pseudo_registers[bb]);
                find_reg((*ptr)->arg3, pseudo_registers[bb]);

                get_special_uses(*ptr, pseudo_registers[bb]);
            } else if(auto* ptr = boost::get<std::shared_ptr<ltac::Jump>>(&statement)){
                get_special_uses(*ptr, pseudo_registers[bb]);
            }
        }
    }

    //TODO Find a more efficient way to prune the results...

    for(auto& bb : function){
        for(auto& reg : pseudo_registers[bb]){
            bool found = false;

            for(auto& bb2 : function){
                if(bb != bb2 && pseudo_registers[bb2].count(reg)){
                    found = true;
                    break;
                }
            }

            if(!found){
                local_pseudo_registers[bb].insert(reg);
            }
        }
    }
}

template<typename Pseudo>
std::size_t count_store_complete(mtac::basic_block_p bb, Pseudo& reg){
    std::size_t count = 0;
    for(auto& statement : bb->l_statements){
        if(is_store_complete<Pseudo>(statement, reg)){
            ++count;
        }
    }

    return count;
}

template<typename Pseudo>
void renumber(mtac::function_p function){
    local_reg<Pseudo> local_pseudo_registers;
    find_local_registers(function, local_pseudo_registers);

    auto current_reg = last_register<Pseudo>(function);

    for(auto& bb :function){
        for(auto& reg : local_pseudo_registers[bb]){
            //No need to renumber bound registers
            if(reg.bound){
                continue;
            }

            auto count = count_store_complete(bb, reg);

            if(count > 1){
                Pseudo target;
                bool start = false;

                for(auto& statement : bb->l_statements){
                    if(is_store_complete<Pseudo>(statement, reg)){
                        target = Pseudo(++current_reg);
                        start = true;

                        //Make sure to replace only the first argument
                        auto instruction = boost::get<std::shared_ptr<ltac::Instruction>>(statement);
                        instruction->arg1 = target;
                    } else {
                        if(start){
                            replace_register(statement, reg, target);
                        }
                    }
                }
            }
        }
    }

    set_last_reg<Pseudo>(function, current_reg);
}

//2. Build

template<typename Opt, typename Pseudo>
void gather_reg(Opt& reg, ltac::interference_graph<Pseudo>& graph){
    if(reg){
        if(auto* ptr = boost::get<Pseudo>(&*reg)){
            graph.gather(*ptr);
        }
    }
}

template<typename Opt, typename Pseudo>
void gather(Opt& arg, ltac::interference_graph<Pseudo>& graph){
    if(arg){
        if(auto* ptr = boost::get<Pseudo>(&*arg)){
            graph.gather(*ptr);
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            gather_reg(ptr->base_register, graph);
            gather_reg(ptr->scaled_register, graph);
        }
    }
}

template<typename Pseudo>
void gather_pseudo_regs(mtac::function_p function, ltac::interference_graph<Pseudo>& graph){
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

template<typename Pseudo, typename Results>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoRegister>::value, std::unordered_set<Pseudo>&>::type get_live_results(Results& results){
    return results.registers;
}

template<typename Pseudo, typename Results>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoFloatRegister>::value, std::unordered_set<Pseudo>&>::type get_live_results(Results& results){
    return results.float_registers;
}

template<typename Pseudo>
void build_interference_graph(ltac::interference_graph<Pseudo>& graph, mtac::function_p function){
    //Init the graph structure with the current size
    gather_pseudo_regs(function, graph);
    graph.build_graph();

    //Return quickly
    if(!graph.size()){
        return;
    }

    ltac::LiveRegistersProblem problem;
    auto live_results = mtac::data_flow(function, problem);

    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            auto& live_registers = get_live_results<Pseudo>(live_results->OUT_LS[statement].values());

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

//3. Coalesce

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoRegister>::value, bool>::type is_copy(ltac::Statement& statement){
    if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        return (*ptr)->op == ltac::Operator::MOV 
            && boost::get<Pseudo>(&*(*ptr)->arg1) 
            && boost::get<Pseudo>(&*(*ptr)->arg2);
    }

    return false;
}

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoFloatRegister>::value, bool>::type is_copy(ltac::Statement& statement){
    if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
        return (*ptr)->op == ltac::Operator::FMOV 
            && boost::get<Pseudo>(&*(*ptr)->arg1) 
            && boost::get<Pseudo>(&*(*ptr)->arg2);
    }

    return false;
}

template<typename Pseudo>
bool coalesce(ltac::interference_graph<Pseudo>& graph, mtac::function_p function){
    local_reg<Pseudo> local_pseudo_registers;
    find_local_registers(function, local_pseudo_registers);
    
    std::unordered_set<Pseudo> prune;
    std::unordered_map<Pseudo, Pseudo> replaces;

    //TODO Instead of pruning dependent registers pairs, update the graph 
    //and continue to avoid too many rebuilding of the graph

    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(is_copy<Pseudo>(statement)){
                auto instruction = boost::get<std::shared_ptr<ltac::Instruction>>(statement);
                auto reg1 = boost::get<Pseudo>(*instruction->arg1);
                auto reg2 = boost::get<Pseudo>(*instruction->arg2);

                if(
                           reg1 != reg2
                        && !reg1.bound && !reg2.bound 
                        && local_pseudo_registers[bb].count(reg1) && local_pseudo_registers[bb].count(reg2) 
                        && !graph.connected(graph.convert(reg1), graph.convert(reg2))
                        && !prune.count(reg1) && !prune.count(reg2))
                {
                    log::emit<Debug>("registers") << "Coalesce " << reg1 << " and " << reg2 << log::endl;

                    replaces[reg1] = reg2;
                    prune.insert(reg1);
                    prune.insert(reg2);

                    ltac::transform_to_nop(instruction);            
                }
            }
        }
    }

    replace_registers(function, replaces);

    return !replaces.empty();
}

//4. Spill costs

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

template<typename Opt, typename Pseudo>
void update_cost_reg(Opt& reg, ltac::interference_graph<Pseudo>& graph, unsigned int depth){
    if(reg){
        if(auto* ptr = boost::get<Pseudo>(&*reg)){
            graph.spill_cost(graph.convert(*ptr)) += load_cost * depth_cost(depth);
        }
    }
}

template<typename Opt, typename Pseudo>
void update_cost(Opt& arg, ltac::interference_graph<Pseudo>& graph, unsigned int depth){
    if(arg){
        if(auto* ptr = boost::get<Pseudo>(&*arg)){
            graph.spill_cost(graph.convert(*ptr)) += load_cost * depth_cost(depth);
        } else if(auto* ptr = boost::get<ltac::Address>(&*arg)){
            update_cost_reg(ptr->base_register, graph, depth);
            update_cost_reg(ptr->scaled_register, graph, depth);
        }
    }
}

template<typename Pseudo>
void estimate_spill_costs(mtac::function_p function, ltac::interference_graph<Pseudo>& graph){
    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                if(ltac::erase_result((*ptr)->op)){
                    if(auto* reg_ptr = boost::get<Pseudo>(&*(*ptr)->arg1)){
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

//5. Simplify

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoRegister>::value, unsigned int>::type number_of_registers(Platform platform){
    auto descriptor = getPlatformDescriptor(platform);
    return descriptor->number_of_registers();
}

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoFloatRegister>::value, unsigned int>::type number_of_registers(Platform platform){
    auto descriptor = getPlatformDescriptor(platform);
    return descriptor->number_of_float_registers();
}

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoRegister>::value, std::vector<unsigned short>>::type hard_registers(Platform platform){
    auto descriptor = getPlatformDescriptor(platform);
    return descriptor->symbolic_registers();
}

template<typename Pseudo>
typename std::enable_if<std::is_same<Pseudo, ltac::PseudoFloatRegister>::value, std::vector<unsigned short>>::type hard_registers(Platform platform){
    auto descriptor = getPlatformDescriptor(platform);
    return descriptor->symbolic_float_registers();
}

template<typename Pseudo>
std::size_t spill_heuristic(ltac::interference_graph<Pseudo>& graph, std::size_t reg){
    return graph.spill_cost(reg) / graph.degree(reg);
}

template<typename Pseudo>
void simplify(ltac::interference_graph<Pseudo>& graph, Platform platform, std::vector<std::size_t>& spilled, std::list<std::size_t>& order){
    std::set<std::size_t> n;
    for(std::size_t r = 0; r < graph.size(); ++r){
        n.insert(r);
    }

    auto K = number_of_registers<Pseudo>(platform);
    log::emit<Trace>("registers") << "Attempt a " << K << "-coloring of the graph" << log::endl;

    while(!n.empty()){
        std::size_t node;
        bool found = false;

        for(auto candidate : n){
            log::emit<Dev>("registers") << "Degree(" << graph.convert(candidate) << ") = " << graph.degree(candidate) << log::endl;
            if(graph.degree(candidate) < K){
                node = candidate;        
                found = true;
                break;
            }
        }
        
        if(!found){
            std::size_t min_cost = std::numeric_limits<std::size_t>::max();

            for(auto candidate : n){
               if(!graph.convert(candidate).bound && spill_heuristic(graph, candidate) < min_cost){
                    min_cost = spill_heuristic(graph, candidate);
                    node = candidate;
               }
            }

            log::emit<Trace>("registers") << "Mark pseudo " << graph.convert(node) << " to be spilled" << log::endl;

            spilled.push_back(node);
        } else {
            order.push_back(node);
        }

        n.erase(node);
        graph.remove_node(node);
    }
}

//6. Select

template<typename Pseudo, typename Hard>
void replace_registers(mtac::function_p function, std::unordered_map<std::size_t, std::size_t>& allocation, ltac::interference_graph<Pseudo>& graph){
    std::unordered_map<Pseudo, Hard> register_allocation;

    for(auto& pair : allocation){
        register_allocation[graph.convert(pair.first)] = {pair.second};
    }

    replace_registers(function, register_allocation);
}

template<typename Pseudo, typename Hard>
void select(ltac::interference_graph<Pseudo>& graph, mtac::function_p function, Platform platform, std::list<std::size_t>& order){
    std::unordered_map<std::size_t, std::size_t> allocation;
    
    auto colors = hard_registers<Pseudo>(platform);

    //Handle bound registers
    auto it = iterate(order);
    while(it.has_next()){
        auto reg = *it;

        if(graph.convert(reg).bound){
            log::emit<Trace>("registers") << "Alloc " << graph.convert(reg).binding << " to pseudo " << graph.convert(reg) << " (bound)" << log::endl;
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
                log::emit<Trace>("registers") << "Alloc " << color << " to pseudo " << graph.convert(reg) << log::endl;
                allocation[reg] = color;
                break;
            }
        }
    }

    for(auto& alloc : allocation){
        function->use(Hard(alloc.second));
    }

    replace_registers<Pseudo, Hard>(function, allocation, graph);
}

//7. Spill code

template<typename Pseudo>
void spill_code(ltac::interference_graph<Pseudo>& graph, mtac::function_p function, std::vector<std::size_t>& spilled){
    auto current_reg = last_register<Pseudo>(function);
    
    for(auto reg : spilled){
        auto pseudo_reg = graph.convert(reg);

        //Allocate stack space for the pseudo reg
        auto position = function->context->stack_position();
        position -= INT->size(function->context->global()->target_platform());
        function->context->set_stack_position(position);

        for(auto& bb : function){
            auto it = iterate(bb->l_statements);
            
            //TODO Add support for spilling float registers

            while(it.has_next()){
                auto statement = *it;

                if(is_store_complete(statement, pseudo_reg)){
                    Pseudo new_pseudo_reg(++current_reg);

                    replace_register(statement, pseudo_reg, new_pseudo_reg);

                    it.insert_after(std::make_shared<ltac::Instruction>(ltac::Operator::MOV, ltac::Address(ltac::BP, position), new_pseudo_reg));
                } else if(is_store(statement, pseudo_reg)){
                    Pseudo new_pseudo_reg(++current_reg);
                    
                    it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::MOV, new_pseudo_reg, ltac::Address(ltac::BP, position)));

                    ++it;
                    
                    it.insert_after(std::make_shared<ltac::Instruction>(ltac::Operator::MOV, ltac::Address(ltac::BP, position), new_pseudo_reg));
                    
                    replace_register(statement, pseudo_reg, new_pseudo_reg);
                } else if(is_load(statement, pseudo_reg)){
                    Pseudo new_pseudo_reg(++current_reg);

                    it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::MOV, new_pseudo_reg, ltac::Address(ltac::BP, position)));

                    ++it;

                    replace_register(statement, pseudo_reg, new_pseudo_reg);
                } 
                
                ++it;
            }
        }
    }

    set_last_reg<Pseudo>(function, current_reg);
}

//Register allocation

template<typename Pseudo, typename Hard>
void register_allocation(mtac::function_p function, Platform platform){
    bool coalesced = false;

    while(true){
        if(!coalesced){
            //1. Renumber
            renumber<Pseudo>(function);
        }

        //2. Build
        ltac::interference_graph<Pseudo> graph;
        build_interference_graph(graph, function);

        //No pseudo registers, return quickly
        if(!graph.size()){
            return;
        }

        //3. Coalesce
        coalesced = coalesce(graph, function);

        if(coalesced){
            continue;
        }

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
            select<Pseudo, Hard>(graph, function, platform, order);

            return;
        }
    }
}

} //end of anonymous namespace

void ltac::register_allocation(std::shared_ptr<mtac::Program> program, Platform platform){
    PerfsTimer timer("Register allocation");

    for(auto& function : program->functions){
        log::emit<Trace>("registers") << "Allocate integer registers for function " << function->getName() << log::endl;
        ::register_allocation<ltac::PseudoRegister, ltac::Register>(function, platform);
        
        log::emit<Trace>("registers") << "Allocate float registers for function " << function->getName() << log::endl;
        ::register_allocation<ltac::PseudoFloatRegister, ltac::FloatRegister>(function, platform);
    }
}
