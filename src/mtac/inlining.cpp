//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>
#include <unordered_map>

#include "SymbolTable.hpp"
#include "Options.hpp"
#include "Type.hpp"
#include "FunctionContext.hpp"
#include "iterators.hpp"

#include "mtac/inlining.hpp"
#include "mtac/Printer.hpp"

using namespace eddic;

std::size_t size_of(std::shared_ptr<mtac::Function> function){
    std::size_t size = 0;

    for(auto block : function->getBasicBlocks()){
        size += block->statements.size();
    }

    return size;
}

typedef std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> VariableClones;
typedef std::unordered_map<std::shared_ptr<mtac::BasicBlock>, std::shared_ptr<mtac::BasicBlock>> BBClones;

template<typename Value>
void update_usage(VariableClones& clones, Value& value){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&value)){
        if(clones.find(*ptr) != clones.end()){
            value = clones[*ptr];
        }
    }
}

template<typename Opt>
void update_usage_optional(VariableClones& clones, Opt& opt){
    if(opt){
        update_usage(clones, *opt);
    }
}

void update_usages(VariableClones& clones, mtac::Statement& statement){
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr; 

        if(clones.find(quadruple->result) != clones.end()){
            quadruple->result = clones[quadruple->result];
        }

        update_usage_optional(clones, quadruple->arg1);
        update_usage_optional(clones, quadruple->arg2);
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
        auto param = *ptr; 
        
        update_usage(clones, param->arg);
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
        auto if_ = *ptr; 
        
        update_usage(clones, if_->arg1);
        update_usage_optional(clones, if_->arg2);
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
        auto if_ = *ptr; 
        
        update_usage(clones, if_->arg1);
        update_usage_optional(clones, if_->arg2);
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
        auto call_ = *ptr; 

        if(call_->return_ && clones.find(call_->return_) != clones.end()){
            call_->return_ = clones[call_->return_];
        }

        if(call_->return2_ && clones.find(call_->return2_) != clones.end()){
            call_->return2_ = clones[call_->return2_];
        }
    }
}

void update_usages(BBClones& clones, mtac::Statement& statement){
    if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
        auto if_ = *ptr; 

        if(clones.find(if_->block) != clones.end()){
            if_->block = clones[if_->block];
        }
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
        auto if_ = *ptr; 

        if(clones.find(if_->block) != clones.end()){
            if_->block = clones[if_->block];
        }
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&statement)){
        auto goto_ = *ptr; 

        if(clones.find(goto_->block) != clones.end()){
            goto_->block = clones[goto_->block];
        }
    }
}

void clone(std::vector<mtac::Statement>& sources, std::vector<mtac::Statement>& destination){
    for(auto& statement : sources){
        if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
            auto quadruple = *ptr; 

            auto copy = std::make_shared<mtac::Quadruple>();

            copy->result = quadruple->result;
            copy->arg1 = quadruple->arg1;
            copy->arg2 = quadruple->arg2;
            copy->op = quadruple->op;

            destination.push_back(copy);
        } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
            auto param = *ptr; 

            auto copy = std::make_shared<mtac::Param>();
            
            copy->arg = param->arg;
            copy->param = param->param;
            copy->std_param = param->std_param;
            copy->function = param->function;
            copy->address = param->address;
            copy->memberNames = param->memberNames;
    
            destination.push_back(copy);
        } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
            auto if_ = *ptr; 
            
            auto copy = std::make_shared<mtac::IfFalse>();
            
            copy->op = if_->op;
            copy->arg1 = if_->arg1;
            copy->arg2 = if_->arg2;
            copy->label = if_->label;
            copy->block = if_->block;
            
            destination.push_back(copy);
        } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
            auto if_ = *ptr; 

            auto copy = std::make_shared<mtac::If>();
            
            copy->op = if_->op;
            copy->arg1 = if_->arg1;
            copy->arg2 = if_->arg2;
            copy->label = if_->label;
            copy->block = if_->block;
            
            destination.push_back(copy);
        } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Goto>>(&statement)){
            auto goto_ = *ptr; 

            auto copy = std::make_shared<mtac::Goto>(goto_->label, goto_->type);
            copy->block = goto_->block;
            destination.push_back(copy);
        } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
            auto call = *ptr; 

            destination.push_back(std::make_shared<mtac::Call>(call->function, call->functionDefinition, call->return_, call->return2_));
        } else if(auto* ptr = boost::get<std::string>(&statement)){
            destination.push_back(*ptr);
        } 

        //No need to copy NOP
    }
}

template<typename Iterator>
BBClones clone(std::shared_ptr<mtac::Function> source_function, std::shared_ptr<mtac::Function> dest_function, Iterator bit){
    BBClones bb_clones;

    for(auto block : source_function->getBasicBlocks()){
        //Copy all basic blocks except ENTRY and EXIT
        if(block->index >= 0){
            auto new_bb = std::make_shared<mtac::BasicBlock>(dest_function->getBasicBlocks().size() + 1);
            new_bb->context = block->context;
            clone(block->statements, new_bb->statements);

            bb_clones[block] = new_bb;

            bit.insert(new_bb);

            ++bit;
        }
    }
    
    return bb_clones;
}

template<typename Iterator>
VariableClones copy_parameters(std::shared_ptr<mtac::Function> source_function, std::shared_ptr<mtac::Function> dest_function, Iterator bit){
    VariableClones variable_clones;

    auto source_definition = source_function->definition;
    auto dest_definition = dest_function->definition;

    if(source_definition->parameters.size() > 0){
        //Param are in the previous block
        --bit;

        auto pit = (*bit)->statements.end() - 1;

        for(int i = source_definition->parameters.size() - 1; i >= 0;){
            auto statement = *pit;

            if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
                auto param = source_definition->parameters[i];

                auto quadruple = std::make_shared<mtac::Quadruple>();

                auto param_var = dest_definition->context->new_temporary(param.paramType);
                variable_clones[(*ptr)->param] = param_var;
                quadruple->result = param_var;

                if(param.paramType == INT || param.paramType == BOOL){
                    quadruple->op = mtac::Operator::ASSIGN; 
                } else {
                    quadruple->op = mtac::Operator::FASSIGN; 
                }

                quadruple->arg1 = (*ptr)->arg;

                *pit = quadruple;

                --i;
            }

            --pit;
        }
    }

    return variable_clones;
}

template<typename Iterator>
void adapt_instructions(VariableClones& variable_clones, BBClones& bb_clones, std::shared_ptr<mtac::Call> call, Iterator bit){
    auto basic_block = *bit;
    --bit;

    auto cloned_bb = bb_clones.size();

    while(cloned_bb > 0){
        auto new_bb = *bit;

        auto ssit = iterate(new_bb->statements);

        while(ssit.has_next()){
            update_usages(variable_clones, *ssit);
            update_usages(bb_clones, *ssit);

            if(auto* ret_ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&*ssit)){
                auto quadruple = *ret_ptr;

                if(quadruple->op == mtac::Operator::RETURN){
                    auto goto_ = std::make_shared<mtac::Goto>();
                    goto_->block = basic_block;

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
        --bit;
    }
}

bool is_recursive(std::shared_ptr<mtac::Function> function){
    for(auto& basic_block : function->getBasicBlocks()){
        for(auto& statement : basic_block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                if((*ptr)->functionDefinition == function->definition){
                    return true;
                }
            }
        }
    }

    return false;
}

bool can_be_inlined(std::shared_ptr<mtac::Function> function){
    //The main function cannot be inlined
    if(function->getName() == "main"){
        return false;
    }

    for(auto& param : function->definition->parameters){
        if(param.paramType != INT && param.paramType != FLOAT && param.paramType != BOOL){
            return false;
        }
    }

    if(is_recursive(function)){
        return false;
    }

    return true;
}

bool will_inline(std::shared_ptr<mtac::Function> function){
    if(can_be_inlined(function)){
        //function called once
        if(symbols.referenceCount(function->getName()) == 1){
            return true;
        } else {
            auto size = size_of(function);

            //Inline little functions
            if(size < 10){
                return true;
            }
        }
    }

    return false;
}

bool mtac::inline_functions(std::shared_ptr<mtac::Program> program){
    if(option_defined("fno-inline")){
        return false;
    }

    bool optimized = false;

    for(auto source_function : program->functions){
        if(!will_inline(source_function)){
            continue;
        }

        auto source_definition = source_function->definition;
       
        for(auto dest_function : program->functions){
            if(dest_function == source_function){
                continue;
            }
        
            //If the function has already been inlined
            if(dest_function->getName() != "main" && symbols.referenceCount(dest_function->getName()) <= 0){
                continue;
            }

            for(auto bit = iterate(dest_function->getBasicBlocks()); bit.has_next(); ++bit){
                auto basic_block = *bit;

                auto it = iterate(basic_block->statements);

                while(it.has_next()){
                    if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&*it)){
                        auto call = *ptr;

                        if(call->functionDefinition == source_definition){
                            //Copy the parameters
                            auto variable_clones = copy_parameters(source_function, dest_function, bit);
                            
                            //Allocate storage for the local variables of the inlined function
                            for(auto variable_pair : source_definition->context->stored_variables()){
                                auto variable = variable_pair.second;
                                variable_clones[variable] = dest_function->definition->context->newVariable(variable);
                            }

                            //Clone all the source basic blocks in the dest function
                            auto bb_clones = clone(source_function, dest_function, bit);

                            //Fix all the instructions (clones and return)
                            adapt_instructions(variable_clones, bb_clones, call, bit);

                            //Erase the original call
                            it.erase();

                            symbols.removeReference(source_function->getName());
                            optimized = true;

                            continue;
                        }
                    }

                    ++it;
                }
            }
        }
    }

    return optimized;
}
