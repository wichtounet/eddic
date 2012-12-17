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
#include "mtac/Statement.hpp"

using namespace eddic;

namespace {

typedef std::unordered_map<mtac::basic_block_p, mtac::basic_block_p> BBClones;

struct BBReplace : public boost::static_visitor<> {
    BBClones& clones;

    BBReplace(BBClones& clones) : clones(clones) {}

    void operator()(std::shared_ptr<mtac::IfFalse> if_){
        if(clones.find(if_->block) != clones.end()){
            if_->block = clones[if_->block];
        }
    }
    
    void operator()(std::shared_ptr<mtac::If> if_){
        if(clones.find(if_->block) != clones.end()){
            if_->block = clones[if_->block];
        }
    }
    
    void operator()(std::shared_ptr<mtac::Goto> goto_){
        if(clones.find(goto_->block) != clones.end()){
            goto_->block = clones[goto_->block];
        }
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

BBClones clone(mtac::function_p source_function, mtac::function_p dest_function, mtac::basic_block_p bb, std::shared_ptr<GlobalContext> context){
    LOG<Trace>("Inlining") << "Clone " << source_function->getName() << " into " << dest_function->getName() << log::endl;

    BBClones bb_clones;

    std::vector<mtac::basic_block_p> cloned;

    auto old_entry = source_function->entry_bb();
    auto old_exit = source_function->exit_bb();

    auto entry = bb->prev;
    auto exit = bb;

    for(auto& block : source_function){
        //Copy all basic blocks except ENTRY and EXIT
        if(block->index >= 0){
            auto new_bb = dest_function->new_bb();

            //Copy the control flow graph properties, they will be corrected after
            new_bb->successors = block->successors;
            new_bb->predecessors = block->predecessors;
    
            for(auto& statement : block->statements){
                new_bb->statements.push_back(mtac::copy(statement, context));
            }

            bb_clones[block] = new_bb;
            cloned.push_back(new_bb);

            dest_function->insert_before(dest_function->at(bb), new_bb);
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

mtac::VariableClones copy_parameters(mtac::function_p source_function, mtac::function_p dest_function, mtac::basic_block_p bb){
    mtac::VariableClones variable_clones;

    auto source_definition = source_function->definition;
    auto dest_definition = dest_function->definition;

    std::unordered_map<std::shared_ptr<Variable>, bool> string_states;

    unsigned int parameters = 0;

    for(auto& param : source_definition->parameters){
        if(param.paramType == STRING){
            parameters += 2;
        } else {
            ++parameters;
        }
    }

    if(source_definition->parameters.size() > 0){
        auto param_bb = bb->prev;

        auto pit = param_bb->statements.end() - 1;

        for(int i = parameters - 1; i >= 0;){
            auto statement = *pit;

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
                auto src_var = (*ptr)->param;

                if(src_var->type()->is_array()){
                    auto dest_var = boost::get<std::shared_ptr<Variable>>((*ptr)->arg);

                    variable_clones[src_var] = dest_var;

                    auto quadruple = std::make_shared<mtac::Quadruple>();
                    quadruple->op = mtac::Operator::NOP;
                    *pit = quadruple;
                } else if(src_var->type() == STRING){
                    if(!string_states.count(src_var)){
                        auto dest_var = dest_definition->context->newVariable(src_var);

                        variable_clones[src_var] = dest_var;

                        //Copy the label
                        auto quadruple = std::make_shared<mtac::Quadruple>();
                        quadruple->op = mtac::Operator::ASSIGN;
                        quadruple->result = dest_var;
                        quadruple->arg1 = (*ptr)->arg;

                        *pit = quadruple;

                        string_states[src_var] = true;
                    } else {
                        auto state = string_states[src_var];

                        if(state){
                            auto dest_var = variable_clones[src_var];

                            //Copy the size
                            auto quadruple = std::make_shared<mtac::Quadruple>();
                            quadruple->op = mtac::Operator::DOT_ASSIGN;
                            quadruple->result = dest_var;
                            quadruple->arg1 = static_cast<int>(INT->size(dest_definition->context->global()->target_platform()));
                            quadruple->arg2 = (*ptr)->arg;

                            *pit = quadruple;

                            string_states[src_var] = false;
                        } else {
                            std::cout << "why ?" << std::endl;
                        }
                    }
                } else {
                    auto quadruple = std::make_shared<mtac::Quadruple>();
                    std::shared_ptr<Variable> dest_var;
                    
                    auto type = src_var->type();

                    dest_var = dest_definition->context->new_temporary(type);

                    if(type == INT || type == BOOL || type == CHAR){
                        quadruple->op = mtac::Operator::ASSIGN; 
                    } else if(type->is_pointer()){
                        quadruple->op = mtac::Operator::PASSIGN;
                    } else {
                        quadruple->op = mtac::Operator::FASSIGN; 
                    }

                    quadruple->arg1 = (*ptr)->arg;
                
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

unsigned int count_constant_parameters(mtac::function_p source_function, mtac::function_p dest_function, mtac::basic_block_p bb){
    unsigned int constant = 0;

    auto source_definition = source_function->definition;
    auto dest_definition = dest_function->definition;

    if(source_definition->parameters.size() > 0){
        auto param_bb = bb->prev;

        auto pit = param_bb->statements.end() - 1;

        for(int i = source_definition->parameters.size() - 1; i >= 0;){
            auto statement = *pit;

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
                auto src_var = (*ptr)->param;

                if(src_var->type()->is_standard_type()){
                    auto arg = (*ptr)->arg;
                
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

void adapt_instructions(mtac::VariableClones& variable_clones, BBClones& bb_clones, std::shared_ptr<mtac::Call> call, mtac::basic_block_p basic_block){
    mtac::VariableReplace variable_replacer(variable_clones);
    BBReplace bb_replacer(bb_clones);

    auto new_bb = basic_block->prev;

    auto cloned_bb = bb_clones.size();

    while(cloned_bb > 0){
        auto ssit = iterate(new_bb->statements);

        while(ssit.has_next()){
            auto statement = *ssit;

            visit(variable_replacer, statement);
            visit(bb_replacer, statement);

            if(auto* ret_ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ret_ptr;

                if(quadruple->op == mtac::Operator::RETURN){
                    auto goto_ = std::make_shared<mtac::Goto>();
                    goto_->block = basic_block;

                    mtac::remove_edge(new_bb, new_bb->next);
                    mtac::make_edge(new_bb, basic_block);

                    if(!call->return_){
                        //If the caller does not care about the return value, return has no effect
                        ssit.erase();
                        ssit.insert(goto_);

                        continue;
                    } else {
                        mtac::Operator op;
                        if(call->functionDefinition->returnType == FLOAT){
                            op = mtac::Operator::FASSIGN;
                        } else if(call->functionDefinition->returnType->is_pointer()){
                            op = mtac::Operator::PASSIGN;
                        } else {
                            op = mtac::Operator::ASSIGN;
                        }

                        if(call->return2_){
                            ssit.insert(std::make_shared<mtac::Quadruple>(call->return2_, *quadruple->arg2, op));

                            ++ssit;
                        }

                        quadruple->op = op;
                        quadruple->result = call->return_;
                        quadruple->arg2.reset();

                        ++ssit;
                        
                        ssit.insert(goto_);

                        continue;
                    }
                }
            }

            ++ssit;
        }

        --cloned_bb;
        new_bb = new_bb->prev;
    }
}

bool can_be_inlined(mtac::function_p function){
    //The main function cannot be inlined
    if(function->getName() == "_F4main" || function->getName() == "_F4mainAS"){
        return false;
    }

    for(auto& param : function->definition->parameters){
        if(!param.paramType->is_standard_type() && !param.paramType->is_pointer() && !param.paramType->is_array()){
            return false;
        }
    }

    if(mtac::is_recursive(function)){
        return false;
    }

    return true;
}

bool will_inline(mtac::function_p source_function, mtac::function_p target_function, std::shared_ptr<mtac::Call> call, mtac::basic_block_p bb){
    //Do not inline recursive calls
    if(source_function == target_function){
        return false;
    }

    if(can_be_inlined(target_function)){
        auto source_size = source_function->size();
        auto target_size = target_function->size();

        auto constant_parameters = count_constant_parameters(target_function, source_function, bb);

        //If all parameters are constant, there are high chances of further optimizations
        if(target_function->definition->parameters.size() == constant_parameters){
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
        if(target_function->context->global()->referenceCount(target_function->getName()) == 1){
            return source_size < 300 && target_size < 100;
        } 

        //Inline little functions
        return target_size < 15 && source_size < 300;
    }

    return false;
}

mtac::function_p get_target(std::shared_ptr<mtac::Call> call, mtac::program_p program){
    auto target_definition = call->functionDefinition;

    for(auto& function : program->functions){
        if(function->definition == target_definition){
            return function;
        }
    }

    return nullptr;
}

bool call_site_inlining(mtac::function_p dest_function, mtac::program_p program){
    bool optimized = false;

    auto bit = dest_function->begin();
    auto bend = dest_function->end();
        
    while(bit != bend){
        auto basic_block = *bit;

        auto it = iterate(basic_block->statements);

        while(it.has_next()){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&*it)){
                auto call = *ptr;

                auto source_function = get_target(call, program);

                //If the function is not present in the program, it's a standard function, not inlinable
                if(!source_function){
                    ++it;
                    continue;
                }

                auto source_definition = source_function->definition;
                auto dest_definition = dest_function->definition;

                if(will_inline(dest_function, source_function, call, basic_block)){
                    LOG<Trace>("Inlining") << "Inline " << source_function->getName() << " into " << dest_function->getName() << log::endl;

                    //Copy the parameters
                    auto variable_clones = copy_parameters(source_function, dest_function, basic_block);

                    //Allocate storage for the local variables of the inlined function
                    for(auto& variable : source_definition->context->stored_variables()){
                        variable_clones[variable] = dest_definition->context->newVariable(variable);
                    }

                    //Clone all the source basic blocks in the dest function
                    auto bb_clones = clone(source_function, dest_function, basic_block, program->context);

                    //Fix all the instructions (clones and return)
                    adapt_instructions(variable_clones, bb_clones, call, basic_block);

                    //Erase the original call
                    it.erase();

                    //The target function is called one less time
                    program->context->removeReference(source_definition->mangledName);
                    optimized = true;

                    continue;
                }
            }

            ++it;
        }

        ++bit;
    }

    return optimized;
}

} //end of anonymous namespace

void mtac::inline_functions::set_configuration(std::shared_ptr<Configuration> configuration){
    this->configuration = configuration;
}

bool mtac::inline_functions::operator()(mtac::program_p program){
    if(configuration->option_defined("fno-inline-functions")){
        return false;
    }

    if(configuration->option_defined("finline-functions")){
        bool optimized = false;
        auto global_context = program->context;

        for(auto& function : program->functions){
            //If the function is never called, no need to optimize it
            if(global_context->referenceCount(function->getName()) <= 0){
                continue; 
            }

            optimized |= call_site_inlining(function, program);
        }

        return optimized;
    } else {
        return false;
    }
}
