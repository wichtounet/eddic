//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>

#include "SymbolTable.hpp"
#include "Options.hpp"
#include "Type.hpp"
#include "FunctionContext.hpp"

#include "mtac/inlining.hpp"

using namespace eddic;

std::size_t size_of(std::shared_ptr<mtac::Function> function){
    std::size_t size = 0;

    for(auto block : function->getBasicBlocks()){
        size += block->statements.size();
    }

    return size;
}

typedef std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> Clones;

template<typename Value>
void update_usage(Clones& clones, Value& value){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&value)){
        if(clones.find(*ptr) != clones.end()){
            value = clones[*ptr];
        }
    }
}

template<typename Opt>
void update_usage_optional(Clones& clones, Opt& opt){
    if(opt){
        update_usage(clones, *opt);
    }
}

void update_usages(Clones& clones, mtac::Statement& statement){
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

bool mtac::inline_functions(std::shared_ptr<mtac::Program> program){
    if(option_defined("fno-inline")){
        return false;
    }

    bool optimized = false;

    std::vector<std::shared_ptr<mtac::Function>> inlined;

    for(auto function : program->functions){
        //The main function cannot be inlined
        if(function->getName() == "main"){
            continue;
        }

        //function called once
        if(symbols.referenceCount(function->getName()) == 1){
            inlined.push_back(function); 
        } else {
            auto size = size_of(function);

            //Inline little functions
            if(size < 10){
                inlined.push_back(function);
            }
        }
    }

    for(auto source_function : inlined){
        auto source_definition = source_function->definition;
       
        for(auto dest_function : program->functions){
            if(dest_function == source_function){
                continue;
            }

            auto bit = dest_function->getBasicBlocks().begin();
            auto bend = dest_function->getBasicBlocks().end();

            while(bit != bend){
                auto basic_block = *bit;

                auto it = basic_block->statements.begin();
                auto end = basic_block->statements.end();

                while(it != end){
                    auto statement = *it;

                    if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                        auto call = *ptr;

                        if(call->functionDefinition == source_definition){
                            //Handle parameters
                            if(source_definition->parameters.size() > 0){
                                ++it;
                                continue;//TODO Temporary
                            }

                            optimized = true;
                            
                            std::cout << "inline " << source_definition->mangledName << " in function " << dest_function->definition->mangledName << std::endl;

                            std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> clones;

                            for(auto variable_pair : *source_definition->context){
                                auto variable = variable_pair.second;
                                auto clone = dest_function->definition->context->newVariable(variable);

                                clones[variable] = clone;

                                std::cout << "Clone variable " << variable->name() << " into " << clone->name() << std::endl;
                            }

                            auto saved_bit = bit;

                            for(auto block : source_function->getBasicBlocks()){
                                //Copy all basic blocks except ENTRY and EXIT
                                if(block->index >= 0){
                                    auto new_bb = std::make_shared<mtac::BasicBlock>(dest_function->getBasicBlocks().size() + 1);
                                    new_bb->context = block->context;
                                    clone(block->statements, new_bb->statements);

                                    if(!clones.empty() || source_definition->returnType != VOID){
                                        auto ssit = new_bb->statements.begin();
                                        auto ssend = new_bb->statements.end();

                                        while(ssit != ssend){
                                            update_usages(clones, *ssit);

                                            if(auto* ret_ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&*ssit)){
                                                auto quadruple = *ret_ptr;

                                                if(quadruple->op == mtac::Operator::RETURN){
                                                    if(!call->return_){
                                                        //If the caller does not care about the return value, return has no effect
                                                        ssit = new_bb->statements.erase(it);
                                                        ssend = new_bb->statements.end();
                                                    } else {
                                                        mtac::Operator op;
                                                        if(source_definition->returnType == FLOAT){
                                                            op = mtac::Operator::FASSIGN;
                                                        } else if(source_definition->returnType->is_pointer()){
                                                            op = mtac::Operator::PASSIGN;
                                                        } else {
                                                            op = mtac::Operator::ASSIGN;
                                                        }

                                                        if(call->return2_){
                                                            auto new_quadruple = std::make_shared<mtac::Quadruple>(call->return2_, *quadruple->arg2, op);

                                                            ssit = new_bb->statements.insert(it, new_quadruple);
                                                            ssend = new_bb->statements.end();
                                                        }
                                                        
                                                        quadruple->op = op;
                                                        quadruple->result = call->return_;
                                                        quadruple->arg2.reset();
                                                    }
                                                }
                                            }

                                            ++ssit;
                                        }
                                    }

                                    bit = dest_function->getBasicBlocks().insert(bit, new_bb);
                                    bend = dest_function->getBasicBlocks().end();

                                    ++bit;
                                }
                            }

                            bit = saved_bit;

                            it = basic_block->statements.erase(it);
                            end = basic_block->statements.end();

                            continue;
                        }
                    }

                    ++it;
                }

                ++bit;
            }
        }
    }

    return optimized;
}
