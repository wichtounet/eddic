//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <vector>
#include <unordered_map>

#include "logging.hpp"
#include "Options.hpp"
#include "Type.hpp"
#include "FunctionContext.hpp"
#include "iterators.hpp"
#include "VisitorUtils.hpp"
#include "GlobalContext.hpp"
#include "Variable.hpp"

#include "mtac/inlining.hpp"
#include "mtac/Utils.hpp"
#include "mtac/VariableReplace.hpp"
#include "mtac/ControlFlowGraph.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

namespace {

mtac::basic_block_p create_safe_block(mtac::Function& dest_function, mtac::basic_block_p bb){
    auto safe_block = dest_function.new_bb();

    //Insert the new basic block before the old one
    dest_function.insert_after(dest_function.at(bb), safe_block);

    safe_block->statements = std::move(bb->statements);

    for(auto succ : bb->successors){
        mtac::make_edge(safe_block, succ);
    }

    while(!bb->successors.empty()){
        mtac::remove_edge(bb, bb->successors[0]);
    }

    mtac::make_edge(bb, safe_block);

    return safe_block;
}

mtac::basic_block_p split_if_necessary(mtac::Function& dest_function, mtac::basic_block_p bb, std::size_t call_uid){
    if(bb->statements.front().uid() == call_uid){
        log::emit<Trace>("Inlining") << "No need to split " << bb << log::endl;

        //Erase the call
        mtac::transform_to_nop(bb->statements.front());

        //The basic block remains the same
        return bb;
    } else {
        log::emit<Trace>("Inlining") << "Split block " << bb << " to perform inlining" << log::endl;

        auto split_block = dest_function.new_bb();

        dest_function.insert_after(dest_function.at(bb), split_block);

        for(auto succ : bb->successors){
            mtac::make_edge(split_block, succ);
        }

        while(!bb->successors.empty()){
            mtac::remove_edge(bb, bb->successors[0]);
        }

        mtac::make_edge(bb, split_block);

        auto pit = bb->statements.begin();

        while(pit->uid() != call_uid){
            ++pit;
        }

        //Erase the call
        mtac::transform_to_nop(*pit);
        ++pit;

        //Transfer the remaining statements to split_block
        split_block->statements.insert(split_block->statements.begin(), pit, bb->statements.end());
        std::for_each(pit, bb->statements.end(), mtac::transform_to_nop);

        return split_block;
    }
}

mtac::BBClones clone(mtac::Function& source_function, mtac::Function& dest_function, mtac::basic_block_p bb){
    LOG<Trace>("Inlining") << "Clone " << source_function.get_name() << " into " << dest_function.get_name() << log::endl;

    mtac::BBClones bb_clones;

    std::vector<mtac::basic_block_p> cloned;

    auto old_entry = source_function.entry_bb();
    auto old_exit = source_function.exit_bb();

    auto entry = bb->prev;
    auto exit = bb;

    for(auto& block : source_function){
        //Copy all basic blocks except ENTRY and EXIT
        if(block->index >= 0){
            auto new_bb = dest_function.new_bb();

            //Copy the control flow graph properties, they will be corrected after
            new_bb->successors = block->successors;
            new_bb->predecessors = block->predecessors;

            for(auto& statement : block->statements){
                new_bb->statements.push_back(mtac::copy(statement));
            }

            bb_clones[block] = new_bb;
            cloned.push_back(new_bb);

            dest_function.insert_before(dest_function.at(exit), new_bb);
        }
    }

    mtac::remove_edge(entry, exit);

    for(auto& block : cloned){
        for(auto& succ : block->successors){
            if(succ == old_exit){
                succ = exit;
                exit->predecessors.push_back(block);
            } else {
                succ = bb_clones[succ];
            }
        }

        for(auto& pred : block->predecessors){
            if(pred == old_entry){
                pred = entry;
                entry->successors.push_back(block);
            } else {
                pred = bb_clones[pred];
            }
        }
    }

    return bb_clones;
}

mtac::VariableClones copy_parameters(mtac::Function& source_function, mtac::Function& dest_function, mtac::basic_block_p bb){
    mtac::VariableClones variable_clones;

    auto& source_definition = source_function.definition();
    auto& dest_definition = dest_function.definition();

    std::unordered_map<std::shared_ptr<Variable>, bool> string_states;

    unsigned int parameters = 0;

    for(auto& param : source_definition.parameters()){
        if(param.type() == STRING){
            parameters += 2;
        } else {
            ++parameters;
        }
    }

    if(source_definition.parameters().size() > 0){
        //We know for sure that that the parameters are in the previous block
        //This is ensured by split_if_necessary
        auto pit = bb->prev->statements.end() - 1;

        for(int i = parameters - 1; i >= 0;){
            auto& statement = *pit;

            if(statement.op == mtac::Operator::PARAM || statement.op == mtac::Operator::PPARAM){
                auto src_var = statement.param();

                if(src_var->type()->is_array()){
                    auto dest_var = boost::get<std::shared_ptr<Variable>>(*statement.arg1);

                    variable_clones[src_var] = dest_var;

                    mtac::transform_to_nop(statement);
                } else if(src_var->type() == STRING){
                    if(!string_states.count(src_var)){
                        auto dest_var = dest_definition.context()->newVariable(src_var);

                        variable_clones[src_var] = dest_var;

                        //Copy the label
                        statement.op = mtac::Operator::ASSIGN;
                        statement.result = dest_var;
                        statement.arg2.reset();

                        string_states[src_var] = true;
                    } else {
                        assert(string_states[src_var]);

                        //Copy the size
                        statement.op = mtac::Operator::DOT_ASSIGN;
                        statement.result = boost::get<std::shared_ptr<Variable>>(variable_clones[src_var]);
                        statement.arg2 = statement.arg1;
                        statement.arg1 = static_cast<int>(INT->size(dest_definition.context()->global()->target_platform()));

                        string_states[src_var] = false;
                    }
                } else {
                    std::shared_ptr<Variable> dest_var;

                    auto type = src_var->type();

                    dest_var = dest_definition.context()->new_temporary(type);

                    if(type == INT || type == BOOL || type == CHAR){
                        statement.op = mtac::Operator::ASSIGN;
                    } else if(type->is_pointer()){
                        statement.op = mtac::Operator::PASSIGN;
                    } else {
                        statement.op = mtac::Operator::FASSIGN;
                    }

                    variable_clones[src_var] = dest_var;
                    statement.result = dest_var;
                    statement.arg2.reset();
                }

                --i;
            }

            --pit;
        }
    }

    return variable_clones;
}

unsigned int count_constant_parameters(mtac::Function& source_function, mtac::Function& /*dest_function*/, mtac::basic_block_p bb, mtac::Quadruple& call){
    unsigned int constant = 0;

    auto& source_definition = source_function.definition();

    if(source_definition.parameters().size() > 0){
        mtac::basic_block::iterator pit;

        if(bb->statements.front() == call){
            pit = bb->prev->statements.end() - 1;
        } else {
            pit = bb->statements.begin();

            while(*pit != call){
                ++pit;
            }
        }

        for(int i = source_definition.parameters().size() - 1; i >= 0;){
            auto& quadruple = *pit;

            if(quadruple.op == mtac::Operator::PARAM || quadruple.op == mtac::Operator::PPARAM){
                auto src_var = quadruple.param();

                if(src_var->type()->is_standard_type()){
                    auto arg = *quadruple.arg1;

                    if(boost::get<int>(&arg)){
                        ++constant;
                    } else if(boost::get<double>(&arg)){
                        ++constant;
                    } else if(boost::get<std::string>(&arg)){
                        ++constant;
                    }
                }

                --i;
            }

            --pit;
        }
    }

    return constant;
}

void adapt_instructions(mtac::VariableClones& variable_clones, mtac::BBClones& bb_clones, mtac::Quadruple& call, mtac::basic_block_p basic_block){
    mtac::VariableReplace variable_replacer(variable_clones);

    auto new_bb = basic_block->prev;

    auto cloned_bb = bb_clones.size();

    while(cloned_bb > 0){
        auto ssit = iterate(new_bb->statements);

        while(ssit.has_next()){
            auto& quadruple = *ssit;

            variable_replacer.replace(quadruple);
            mtac::replace_bbs(bb_clones, quadruple);

            if(quadruple.op == mtac::Operator::RETURN){
                auto label = "label";
                mtac::Quadruple goto_(static_cast<const std::string&>(label), mtac::Operator::GOTO);
                goto_.block = basic_block;

                mtac::remove_edge(new_bb, new_bb->next);
                mtac::make_edge(new_bb, basic_block);

                if(!call.return1()){
                    //If the caller does not care about the return value, return has no effect
                    *ssit = std::move(goto_);

                    continue;
                } else {
                    mtac::Operator op;
                    if(call.function().return_type() == FLOAT){
                        op = mtac::Operator::FASSIGN;
                    } else if(call.function().return_type()->is_pointer()){
                        op = mtac::Operator::PASSIGN;
                    } else {
                        op = mtac::Operator::ASSIGN;
                    }

                    if(call.return2()){
                        mtac::Quadruple second(call.return2(), *quadruple.arg2, op);

                        quadruple.op = op;
                        quadruple.result = call.return1();
                        quadruple.arg2.reset();

                        ssit.insert(second);
                        ++ssit;
                    } else {
                        quadruple.op = op;
                        quadruple.result = call.return1();
                        quadruple.arg2.reset();
                    }

                    ++ssit;

                    ssit.insert(std::move(goto_));

                    continue;
                }
            }

            ++ssit;
        }

        --cloned_bb;
        new_bb = new_bb->prev;
    }
}

bool can_be_inlined(mtac::Function& function){
    //The main function cannot be inlined
    if(function.is_main()){
        return false;
    }

    for(auto& param : function.definition().parameters()){
        if(!param.type()->is_standard_type() && !param.type()->is_pointer() && !param.type()->is_array()){
            return false;
        }
    }

    if(mtac::is_recursive(function)){
        return false;
    }

    return true;
}

const int SMALL_FUNCTION = 12;

bool will_inline(mtac::Program& program, mtac::Function& source_function, mtac::Function& target_function, mtac::Quadruple& call, mtac::basic_block_p bb){
    //Do not inline recursive calls
    if(source_function.get_name() == target_function.get_name()){
        return false;
    }

    if(can_be_inlined(target_function)){
        auto caller_size = source_function.size();
        auto callee_size = target_function.size();

        auto constant_parameters = count_constant_parameters(target_function, source_function, bb, call);

        //If all parameters are constant, there are high chances of further optimizations
        if(target_function.definition().parameters().size() == constant_parameters){
            return callee_size < 100 && caller_size < 300;
        }

        //For inner loop, increase the chances of inlining
        if(bb->depth > 1){
            return caller_size < 250 && callee_size < 75;
        }

        //For single loop, increase a bit the changes of inlining
        if(bb->depth > 0){
            return caller_size < 150 && callee_size < 50;
        }

        //function called once
        if(program.call_graph.node(target_function.definition())->in_edges.size() == 1){
            return caller_size < 100 && callee_size < 100;
        }

        return callee_size < SMALL_FUNCTION && caller_size < 200;
    }

    return false;
}

bool non_standard_target(mtac::Quadruple& call, mtac::Program& program){
    auto& target_definition = call.function();

    for(auto& function : program.functions){
        if(function.definition().mangled_name() == target_definition.mangled_name()){
            return false;
        }
    }

    return true;
}

bool call_site_inlining(mtac::Function& dest_function, mtac::Function& source_function, mtac::Program& program){
    auto bit = dest_function.begin();
    auto bend = dest_function.end();

    auto& source_definition = source_function.definition();
    auto& dest_definition = dest_function.definition();

    while(bit != bend){
        auto basic_block = *bit;

        auto it = iterate(basic_block->statements);

        while(it.has_next()){
            auto& src_call = *it;
            if(src_call.op == mtac::Operator::CALL){
                if(non_standard_target(src_call, program)){
                    ++it;
                    continue;
                }

                if(src_call.function() == source_definition){
                    if(will_inline(program, dest_function, source_function, src_call, basic_block)){
                        auto call = src_call;
                        auto src_call_uid = src_call.uid();

                        LOG<Trace>("Inlining") << "Inline " << source_function.get_name() << " into " << dest_function.get_name() << log::endl;
                        source_function.context->global()->stats().inc_counter("inlined_functions");

                        basic_block = split_if_necessary(dest_function, basic_block, src_call_uid);

                        //Copy the parameters
                        auto variable_clones = copy_parameters(source_function, dest_function, basic_block);

                        //Allocate storage for the local variables of the inlined function
                        for(auto& variable : source_definition.context()->stored_variables()){
                            variable_clones[variable] = dest_definition.context()->newVariable(variable);
                        }

                        auto safe = create_safe_block(dest_function, basic_block);

                        //Clone all the source basic blocks in the dest function
                        auto bb_clones = clone(source_function, dest_function, safe);

                        //Fix all the instructions (clones and return)
                        adapt_instructions(variable_clones, bb_clones, call, safe);

                        //The target function is called one less time
                        --program.call_graph.edge(dest_definition, source_definition)->count;

                        //There are perhaps new references to functions
                        for(auto& block : source_function){
                            for(auto& statement : block){
                                if(statement.op == mtac::Operator::CALL){
                                    program.call_graph.add_edge(dest_definition, statement.function());
                                }
                            }
                        }

                        //All the iterators are invalidated at this point
                        //The loop will be restarted
                        return true;
                    }
                }
            }

            ++it;
        }

        ++bit;
    }

    return false;
}

} //end of anonymous namespace

bool mtac::inline_functions::gate(std::shared_ptr<Configuration> configuration){
    if(configuration->option_defined("fno-inline-functions")){
        return false;
    }

    return configuration->option_defined("finline-functions");
}

bool mtac::inline_functions::operator()(mtac::Program& program){
    bool optimized = false;
    auto global_context = program.context;

    auto& call_graph = program.call_graph;

    auto order = call_graph.topological_order();
    call_graph.compute_reachable();

    typedef std::reference_wrapper<eddic::Function> func_ref;

    for(auto& function_ref : order){
        auto& function = function_ref.get();

        //Consider only reachable functions
        if(call_graph.is_reachable(function)){
            //Standard function are assembly functions, cannot be inlined
            if(!function.standard()){
                auto cg_node = call_graph.node(function);

                //Collect the callers of the functions

                std::vector<func_ref> callers;
                for(auto& in_edge : cg_node->in_edges){
                    if(in_edge->count > 0){
                        callers.push_back(in_edge->source->function);
                    }
                }

                //Sort them to inline the edges in the order of the topological sort

                std::sort(callers.begin(), callers.end(),
                        [&order](const func_ref& lhs, const func_ref& rhs){ return std::find(order.begin(), order.end(), lhs) < std::find(order.begin(),order.end(), rhs); });

                auto& source_function = program.mtac_function(function);

                for(auto& caller : callers){
                    if(call_graph.is_reachable(caller.get())){
                        auto& dest_function = program.mtac_function(caller.get());

                        bool local = false;
                        do {
                            local = call_site_inlining(dest_function, source_function, program);
                            optimized |= local;
                        } while(local);
                    }
                }
            }
        }
    }

    call_graph.release_reachable();

    return optimized;
}
