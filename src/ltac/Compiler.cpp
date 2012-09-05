//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Labels.hpp"
#include "FunctionContext.hpp"
#include "VisitorUtils.hpp"
#include "Platform.hpp"
#include "Type.hpp"
#include "PerfsTimer.hpp"
#include "Options.hpp"

#include "ltac/Compiler.hpp"
#include "ltac/StatementCompiler.hpp"
#include "ltac/Utils.hpp"

#include "mtac/Utils.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"

using namespace eddic;

void ltac::Compiler::compile(std::shared_ptr<mtac::Program> source, std::shared_ptr<ltac::Program> target, std::shared_ptr<FloatPool> float_pool, Platform platform){
    target->context = source->context;

    for(auto& src_function : source->functions){
        auto target_function = std::make_shared<ltac::Function>(src_function->context, src_function->getName());
        target_function->definition = src_function->definition;

        target->functions.push_back(target_function);

        compile(src_function, target_function, float_pool, platform);
    }
}

void ltac::Compiler::compile(std::shared_ptr<mtac::Function> src_function, std::shared_ptr<ltac::Function> target_function, std::shared_ptr<FloatPool> float_pool, Platform platform){
    PerfsTimer timer("LTAC Compilation");
    
    //Compute the block usage (in order to know if we have to output the label)
    mtac::computeBlockUsage(src_function, block_usage);

    resetNumbering();

    //First we computes a label for each basic block
    for(auto block : src_function->getBasicBlocks()){
        block->label = newLabel();
    }
    
    PlatformDescriptor* descriptor = getPlatformDescriptor(platform);

    std::vector<ltac::Register> registers;
    auto symbolic_registers = descriptor->symbolic_registers();
    for(auto reg : symbolic_registers){
        registers.push_back({reg});
    }
    
    std::vector<ltac::FloatRegister> float_registers;
    auto float_symbolic_registers = descriptor->symbolic_float_registers();
    for(auto reg : float_symbolic_registers){
        float_registers.push_back({reg});
    }

    auto compiler = std::make_shared<StatementCompiler>(registers, float_registers, target_function, float_pool);
    compiler->manager.compiler = compiler;
    compiler->descriptor = getPlatformDescriptor(platform);

    auto size = src_function->context->size();

    //Enter stack frame
    if(!option_defined("fomit-frame-pointer")){
        ltac::add_instruction(target_function, ltac::Operator::ENTER);
    }

    //Alloc stack space for locals
    ltac::add_instruction(target_function, ltac::Operator::SUB, ltac::SP, size);
    compiler->bp_offset += size;
    
    auto iter = src_function->context->begin();
    auto end = src_function->context->end();

    for(; iter != end; iter++){
        auto var = iter->second;

        //ONly stack variables needs to be cleared
        if(var->position().isStack()){
            auto type = var->type();
            int position = var->position().offset();

            if(type->is_array()){
                ltac::add_instruction(target_function, ltac::Operator::MOV, compiler->stack_address(position), static_cast<int>(type->elements()));
                ltac::add_instruction(target_function, ltac::Operator::MEMSET, compiler->stack_address(position + INT->size(platform)), static_cast<int>((type->data_type()->size(platform) / INT->size(platform) * type->elements())));
            } else if(type->is_custom_type()){
                ltac::add_instruction(target_function, ltac::Operator::MEMSET, compiler->stack_address(position), static_cast<int>(type->size(platform) / INT->size(platform)));
            }
        }
    }
    
    //Compute Liveness
    mtac::LiveVariableAnalysisProblem problem;
    compiler->manager.liveness = mtac::data_flow(src_function, problem);
    compiler->manager.pointer_escaped = problem.pointer_escaped;

    //Then we compile each of them
    for(auto block : src_function->getBasicBlocks()){
        //If necessary add a label for the block
        if(block_usage.find(block) != block_usage.end()){
            (*compiler)(block->label);
        }
    
        //Handle parameters and register-allocated variables
        compiler->reset();
        compiler->collect_parameters(src_function->definition);
        compiler->collect_variables(src_function->definition);
    
        for(unsigned int i = 0; i < block->statements.size(); ++i){
            auto& statement = block->statements[i];

            visit(*compiler, statement);
        }

        //end basic block
        if(!compiler->ended){
            compiler->end_basic_block();
        }
    }
    
    ltac::add_instruction(target_function, ltac::Operator::ADD, ltac::SP, size);
    compiler->bp_offset -= size;
        
    //Leave stack frame
    if(!option_defined("fomit-frame-pointer")){
        ltac::add_instruction(target_function, ltac::Operator::LEAVE);
    }

    ltac::add_instruction(target_function, ltac::Operator::RET);
}
