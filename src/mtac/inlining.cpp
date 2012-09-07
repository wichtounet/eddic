//=======================================================================
// Copyright Baptiste Wicht 2011.
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

#include "mtac/inlining.hpp"
#include "mtac/Printer.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

namespace {

typedef std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> VariableClones;
typedef std::unordered_map<std::shared_ptr<mtac::BasicBlock>, std::shared_ptr<mtac::BasicBlock>> BBClones;

struct VariableReplace : public boost::static_visitor<> {
    VariableClones& clones;

    VariableReplace(VariableClones& clones) : clones(clones) {}

    template<typename Value>
    void update_usage(Value& value){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&value)){
            if(clones.find(*ptr) != clones.end()){
                value = clones[*ptr];
            }
        }
    }

    template<typename Opt>
    void update_usage_optional(Opt& opt){
        if(opt){
            update_usage(*opt);
        }
    }
    
    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        if(clones.find(quadruple->result) != clones.end()){
            quadruple->result = clones[quadruple->result];
        }

        update_usage_optional(quadruple->arg1);
        update_usage_optional(quadruple->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::Param> param){
        update_usage(param->arg);
    }
    
    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        update_usage(if_false->arg1);
        update_usage_optional(if_false->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::If> if_){
        update_usage(if_->arg1);
        update_usage_optional(if_->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::Call> call_){
        if(call_->return_ && clones.find(call_->return_) != clones.end()){
            call_->return_ = clones[call_->return_];
        }

        if(call_->return2_ && clones.find(call_->return2_) != clones.end()){
            call_->return2_ = clones[call_->return2_];
        }
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

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

struct StatementClone : public boost::static_visitor<mtac::Statement> {
    std::shared_ptr<GlobalContext> global_context;

    StatementClone(std::shared_ptr<GlobalContext> global_context) : global_context(global_context) {}

    mtac::Statement operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        auto copy = std::make_shared<mtac::Quadruple>();

        copy->result = quadruple->result;
        copy->arg1 = quadruple->arg1;
        copy->arg2 = quadruple->arg2;
        copy->op = quadruple->op;
        
        return copy;
    }
    
    mtac::Statement operator()(std::shared_ptr<mtac::Param> param){
        auto copy = std::make_shared<mtac::Param>();

        copy->arg = param->arg;
        copy->param = param->param;
        copy->std_param = param->std_param;
        copy->function = param->function;
        copy->address = param->address;

        return copy;
    }

    mtac::Statement operator()(std::shared_ptr<mtac::IfFalse> if_){
        auto copy = std::make_shared<mtac::IfFalse>();

        copy->op = if_->op;
        copy->arg1 = if_->arg1;
        copy->arg2 = if_->arg2;
        copy->label = if_->label;
        copy->block = if_->block;

        return copy;
    }

    mtac::Statement operator()(std::shared_ptr<mtac::If> if_){
        auto copy = std::make_shared<mtac::If>();

        copy->op = if_->op;
        copy->arg1 = if_->arg1;
        copy->arg2 = if_->arg2;
        copy->label = if_->label;
        copy->block = if_->block;

        return copy;
    }
    
    mtac::Statement operator()(std::shared_ptr<mtac::Call> call){
        global_context->addReference(call->functionDefinition->mangledName);

        return std::make_shared<mtac::Call>(call->function, call->functionDefinition, call->return_, call->return2_);
    }

    mtac::Statement operator()(std::shared_ptr<mtac::Goto> goto_){
        auto copy = std::make_shared<mtac::Goto>(goto_->label, goto_->type);
        copy->block = goto_->block;
        return copy;
    }

    mtac::Statement operator()(std::shared_ptr<mtac::NoOp>){
        return std::make_shared<mtac::NoOp>();
    }

    mtac::Statement operator()(std::string& str){
        return str;
    }
};

template<typename Iterator>
BBClones clone(std::shared_ptr<mtac::Function> source_function, std::shared_ptr<mtac::Function> dest_function, Iterator bit, std::shared_ptr<GlobalContext> context){
    log::emit<Trace>("Inlining") << "Clone " << source_function->getName() << " into " << dest_function->getName() << log::endl;

    BBClones bb_clones;
    StatementClone cloner(context);

    for(auto block : source_function->getBasicBlocks()){
        //Copy all basic blocks except ENTRY and EXIT
        if(block->index >= 0){
            auto new_bb = std::make_shared<mtac::BasicBlock>(dest_function->getBasicBlocks().size() + 1);
            new_bb->context = block->context;
    
            for(auto& statement : block->statements){
                new_bb->statements.push_back(visit(cloner, statement));
            }

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

    std::unordered_map<std::shared_ptr<Variable>, bool> string_states;

    unsigned int parameters = 0;

    for(auto param : source_definition->parameters){
        if(param.paramType == STRING){
            parameters += 2;
        } else {
            ++parameters;
        }
    }

    if(source_definition->parameters.size() > 0){
        //Param are in the previous block
        --bit;

        auto pit = (*bit)->statements.end() - 1;

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

template<typename Iterator>
unsigned int count_constant_parameters(std::shared_ptr<mtac::Function> source_function, std::shared_ptr<mtac::Function> dest_function, Iterator bit){
    unsigned int constant = 0;

    auto source_definition = source_function->definition;
    auto dest_definition = dest_function->definition;

    if(source_definition->parameters.size() > 0){
        //Param are in the previous block
        --bit;

        auto pit = (*bit)->statements.end() - 1;

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

template<typename Iterator>
void adapt_instructions(VariableClones& variable_clones, BBClones& bb_clones, std::shared_ptr<mtac::Call> call, Iterator bit){
    VariableReplace variable_replacer(variable_clones);
    BBReplace bb_replacer(bb_clones);

    auto basic_block = *bit;
    --bit;

    auto cloned_bb = bb_clones.size();

    while(cloned_bb > 0){
        auto new_bb = *bit;

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

bool can_be_inlined(std::shared_ptr<mtac::Function> function){
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

template<typename Iterator>
bool will_inline(std::shared_ptr<mtac::Function> source_function, std::shared_ptr<mtac::Function> target_function, std::shared_ptr<mtac::Call> call, Iterator bit){
    //Do not inline recursive calls
    if(source_function == target_function){
        return false;
    }

    if(can_be_inlined(target_function)){
        auto source_size = source_function->size();
        auto target_size = target_function->size();

        auto constant_parameters = count_constant_parameters(target_function, source_function, bit);

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

std::shared_ptr<mtac::Function> get_target(std::shared_ptr<mtac::Call> call, std::shared_ptr<mtac::Program> program){
    auto target_definition = call->functionDefinition;

    for(auto& function : program->functions){
        if(function->definition == target_definition){
            return function;
        }
    }

    return nullptr;
}

bool call_site_inlining(std::shared_ptr<mtac::Function> dest_function, std::shared_ptr<mtac::Program> program){
    bool optimized = false;
        
    for(auto bit = iterate(dest_function->getBasicBlocks()); bit.has_next(); ++bit){
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

                if(will_inline(dest_function, source_function, call, bit)){
                    log::emit<Trace>("Inlining") << "Inline " << source_function->getName() << " into " << dest_function->getName() << log::endl;

                    //Copy the parameters
                    auto variable_clones = copy_parameters(source_function, dest_function, bit);

                    //Allocate storage for the local variables of the inlined function
                    for(auto variable_pair : source_definition->context->stored_variables()){
                        auto variable = variable_pair.second;
                        variable_clones[variable] = dest_definition->context->newVariable(variable);
                    }

                    //Clone all the source basic blocks in the dest function
                    auto bb_clones = clone(source_function, dest_function, bit, program->context);

                    //Fix all the instructions (clones and return)
                    adapt_instructions(variable_clones, bb_clones, call, bit);

                    //Erase the original call
                    it.erase();

                    //The target function is called one less time
                    program->context->removeReference(source_function->getName());
                    optimized = true;

                    continue;
                }
            }

            ++it;
        }
    }

    return optimized;
}

} //end of anonymous namespace

bool mtac::inline_functions(std::shared_ptr<mtac::Program> program, std::shared_ptr<Configuration> configuration){
    if(configuration->option_defined("fno-inline-functions")){
        return false;
    }

    if(configuration->option_defined("finline-functions")){
        bool optimized = false;
        auto global_context = program->context;

        for(auto function : program->functions){
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
