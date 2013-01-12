//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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

typedef std::unordered_map<mtac::basic_block_p, mtac::basic_block_p> BBClones;

struct BBReplace {
    BBClones& clones;

    BBReplace(BBClones& clones) : clones(clones) {}

    void operator()(std::shared_ptr<mtac::Quadruple> goto_){
        if(clones.find(goto_->block) != clones.end()){
            goto_->block = clones[goto_->block];
        }
    }
};

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

mtac::basic_block_p split_if_necessary(mtac::Function& dest_function, mtac::basic_block_p bb, std::shared_ptr<mtac::Quadruple> call){
    if(bb->statements.front() == call){
        //Erase the call
        bb->statements.erase(bb->statements.begin());

        //The basic block remains the same
        return bb;
    } else {
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

        while(*pit != call){
            ++pit;
        }

        //Erase the call
        pit = bb->statements.erase(pit);

        //Transfer the remaining statements to split_block
        split_block->statements.insert(split_block->statements.begin(), pit, bb->statements.end()); 
        bb->statements.erase(pit, bb->statements.end());

        return split_block;
    }
}

BBClones clone(mtac::Function& source_function, mtac::Function& dest_function, mtac::basic_block_p bb){
    LOG<Trace>("Inlining") << "Clone " << source_function.get_name() << " into " << dest_function.get_name() << log::endl;

    BBClones bb_clones;

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

mtac::VariableClones copy_parameters(mtac::Function& source_function, mtac::Function& dest_function, mtac::basic_block_p bb, std::shared_ptr<mtac::Quadruple> call){
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
        eddic_assert(bb->statements.front() == call, "This should have been ensured by split_if_necessary");
        auto pit = bb->prev->statements.end() - 1;

        for(int i = parameters - 1; i >= 0;){
            auto statement = *pit;

            if(statement->op == mtac::Operator::PARAM){
                auto src_var = statement->param();

                if(src_var->type()->is_array()){
                    auto dest_var = boost::get<std::shared_ptr<Variable>>(*statement->arg1);

                    variable_clones[src_var] = dest_var;

                    auto quadruple = std::make_shared<mtac::Quadruple>();
                    quadruple->op = mtac::Operator::NOP;
                    *pit = quadruple;
                } else if(src_var->type() == STRING){
                    if(!string_states.count(src_var)){
                        auto dest_var = dest_definition.context()->newVariable(src_var);

                        variable_clones[src_var] = dest_var;

                        //Copy the label
                        auto quadruple = std::make_shared<mtac::Quadruple>();
                        quadruple->op = mtac::Operator::ASSIGN;
                        quadruple->result = dest_var;
                        quadruple->arg1 = *statement->arg1;

                        *pit = quadruple;

                        string_states[src_var] = true;
                    } else {
                        assert(string_states[src_var]);

                        //Copy the size
                        auto quadruple = std::make_shared<mtac::Quadruple>();
                        quadruple->op = mtac::Operator::DOT_ASSIGN;
                        quadruple->result = boost::get<std::shared_ptr<Variable>>(variable_clones[src_var]);
                        quadruple->arg1 = static_cast<int>(INT->size(dest_definition.context()->global()->target_platform()));
                        quadruple->arg2 = *statement->arg1;

                        *pit = quadruple;

                        string_states[src_var] = false;
                    }
                } else {
                    auto quadruple = std::make_shared<mtac::Quadruple>();
                    std::shared_ptr<Variable> dest_var;

                    auto type = src_var->type();

                    dest_var = dest_definition.context()->new_temporary(type);

                    if(type == INT || type == BOOL || type == CHAR){
                        quadruple->op = mtac::Operator::ASSIGN; 
                    } else if(type->is_pointer()){
                        quadruple->op = mtac::Operator::PASSIGN;
                    } else {
                        quadruple->op = mtac::Operator::FASSIGN; 
                    }

                    quadruple->arg1 = *statement->arg1;

                    variable_clones[src_var] = dest_var;
                    quadruple->result = dest_var;

                    *pit = quadruple;
                }

                --i;
            }

            --pit;
        }
    }

    return variable_clones;
}

unsigned int count_constant_parameters(mtac::Function& source_function, mtac::Function& /*dest_function*/, mtac::basic_block_p bb, std::shared_ptr<mtac::Quadruple> call){
    unsigned int constant = 0;

    auto& source_definition = source_function.definition();

    if(source_definition.parameters().size() > 0){
        mtac::basic_block::iterator pit;
        
        std::cout << "---" << std::endl;
        std::cout << call << std::endl;
        std::cout << bb << std::endl;
        std::cout << bb->statements.size() << std::endl;
        std::cout << bb->statements.front() << std::endl;
        if(bb->statements.front() == call){
            pit = bb->prev->statements.end() - 1;
        } else {
            pit = bb->statements.begin();

            while(*pit != call){
                ++pit;
            }
        }

        for(int i = source_definition.parameters().size() - 1; i >= 0;){
            auto quadruple = *pit;

            if(quadruple->op == mtac::Operator::PARAM){
                auto src_var = quadruple->param();

                if(src_var->type()->is_standard_type()){
                    auto arg = *quadruple->arg1;

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

void adapt_instructions(mtac::VariableClones& variable_clones, BBClones& bb_clones, std::shared_ptr<mtac::Quadruple> call, mtac::basic_block_p basic_block){
    mtac::VariableReplace variable_replacer(variable_clones);
    BBReplace bb_replacer(bb_clones);

    auto new_bb = basic_block->prev;

    auto cloned_bb = bb_clones.size();

    while(cloned_bb > 0){
        auto ssit = iterate(new_bb->statements);

        while(ssit.has_next()){
            auto quadruple = *ssit;

            variable_replacer.replace(quadruple);
            bb_replacer(quadruple);

            if(quadruple->op == mtac::Operator::RETURN){
                auto label = "label";
                auto goto_ = std::make_shared<mtac::Quadruple>(static_cast<const std::string&>(label), mtac::Operator::GOTO);
                goto_->block = basic_block;

                mtac::remove_edge(new_bb, new_bb->next);
                mtac::make_edge(new_bb, basic_block);

                if(!call->return1()){
                    //If the caller does not care about the return value, return has no effect
                    ssit.erase();
                    ssit.insert(goto_);

                    continue;
                } else {
                    mtac::Operator op;
                    if(call->function().return_type() == FLOAT){
                        op = mtac::Operator::FASSIGN;
                    } else if(call->function().return_type()->is_pointer()){
                        op = mtac::Operator::PASSIGN;
                    } else {
                        op = mtac::Operator::ASSIGN;
                    }

                    if(call->return2()){
                        ssit.insert(std::make_shared<mtac::Quadruple>(call->return2(), *quadruple->arg2, op));

                        ++ssit;
                    }

                    quadruple->op = op;
                    quadruple->result = call->return1();
                    quadruple->arg2.reset();

                    ++ssit;

                    ssit.insert(goto_);

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
    if(function.get_name() == "_F4main" || function.get_name() == "_F4mainAS"){
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

bool will_inline(mtac::Function& source_function, mtac::Function& target_function, std::shared_ptr<mtac::Quadruple> call, mtac::basic_block_p bb){
    //Do not inline recursive calls
    if(source_function.get_name() == target_function.get_name()){
        return false;
    }

    if(can_be_inlined(target_function)){
        auto source_size = source_function.size();
        auto target_size = target_function.size();

        auto constant_parameters = count_constant_parameters(target_function, source_function, bb, call);

        //If all parameters are constant, there are high chances of further optimizations
        if(target_function.definition().parameters().size() == constant_parameters){
            return target_size < 250;
        }

        //For inner loop, increase the chances of inlining
        if(call->depth > 1){
            return source_size < 500 && target_size < 100;
        }
        
        //For single loop, increase a bit the changes of inlining
        if(call->depth > 0){
            return source_size < 300 && target_size < 75;
        }

        //function called once
        if(target_function.context->global()->referenceCount(target_function.get_name()) == 1){
            return source_size < 300 && target_size < 100;
        } 

        //Inline little functions
        return target_size < 15 && source_size < 300;
    }

    return false;
}

bool non_standard_target(std::shared_ptr<mtac::Quadruple> call, mtac::Program& program){
    auto& target_definition = call->function();

    for(auto& function : program.functions){
        if(function.definition().mangled_name() == target_definition.mangled_name()){
            return false;
        }
    }

    return true;
}

mtac::Function& get_target(std::shared_ptr<mtac::Quadruple> call, mtac::Program& program){
    auto& target_definition = call->function();

    for(auto& function : program.functions){
        if(function.definition().mangled_name() == target_definition.mangled_name()){
            return function;
        }
    }

    eddic_unreachable("Should not happen");
}

bool call_site_inlining(mtac::Function& dest_function, mtac::Program& program){
    auto bit = dest_function.begin();
    auto bend = dest_function.end();
        
    while(bit != bend){
        auto basic_block = *bit;

        auto it = iterate(basic_block->statements);

        while(it.has_next()){
            auto call = *it;
            if(call->op == mtac::Operator::CALL){
                if(non_standard_target(call, program)){
                    ++it;
                    continue;
                }

                auto& source_function = get_target(call, program);

                std::cout << source_function.get_name() << std::endl;

                auto& source_definition = source_function.definition();
                auto& dest_definition = dest_function.definition();

                if(will_inline(dest_function, source_function, call, basic_block)){
                    LOG<Trace>("Inlining") << "Inline " << source_function.get_name() << " into " << dest_function.get_name() << log::endl;

                    //Copy the parameters
                    auto variable_clones = copy_parameters(source_function, dest_function, basic_block, call);

                    //Allocate storage for the local variables of the inlined function
                    for(auto& variable : source_definition.context()->stored_variables()){
                        variable_clones[variable] = dest_definition.context()->newVariable(variable);
                    }

                    basic_block = split_if_necessary(dest_function, basic_block, call);

                    auto safe = create_safe_block(dest_function, basic_block);

                    //Clone all the source basic blocks in the dest function
                    auto bb_clones = clone(source_function, dest_function, safe);

                    //Fix all the instructions (clones and return)
                    adapt_instructions(variable_clones, bb_clones, call, safe);

                    //The target function is called one less time
                    program.context->removeReference(source_definition.mangled_name());

                    //All the iterators are invalidated at this point
                    //The loop will be restarted
                    return true;
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

    for(auto& function : program.functions){
        //If the function is never called, no need to optimize it
        if(global_context->referenceCount(function.get_name()) <= 0){
            continue; 
        }

        bool local = false;
        do {
            local = call_site_inlining(function, program);
            optimized |= local;
        } while(local);
    }

    return optimized;
}
