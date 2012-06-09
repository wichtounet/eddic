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
#include "Types.hpp"

#include "ltac/Compiler.hpp"
#include "ltac/StatementCompiler.hpp"
#include "ltac/Utils.hpp"

#include "mtac/Utils.hpp"

using namespace eddic;

void ltac::Compiler::compile(std::shared_ptr<mtac::Program> source, std::shared_ptr<ltac::Program> target, std::shared_ptr<FloatPool> float_pool){
    target->context = source->context;

    for(auto& src_function : source->functions){
        auto target_function = std::make_shared<ltac::Function>(src_function->context, src_function->getName());

        target->functions.push_back(target_function);

        compile(src_function, target_function, float_pool);
    }
}

void ltac::Compiler::compile(std::shared_ptr<mtac::Function> src_function, std::shared_ptr<ltac::Function> target_function, std::shared_ptr<FloatPool> float_pool){
    auto size = src_function->context->size();
    
    //Only if necessary, allocates size on the stack for the local variables
    if(size > 0){
        ltac::add_instruction(target_function, ltac::Operator::ALLOC_STACK, size);
    }
    
    auto iter = src_function->context->begin();
    auto end = src_function->context->end();

    for(; iter != end; iter++){
        auto var = iter->second;
        if(var->type()->is_array() && var->position().isStack()){
            int position = -var->position().offset();

            ltac::add_instruction(target_function, ltac::Operator::MOV, ltac::Address(ltac::BP, position), static_cast<int>(var->type()->elements()));

            if(var->type()->element_type() == INT){
                ltac::add_instruction(target_function, ltac::Operator::MEMSET, ltac::Address(ltac::BP, position - 8), static_cast<int>(var->type()->elements()));
            } else if(var->type()->element_type() == STRING){
                ltac::add_instruction(target_function, ltac::Operator::MEMSET, ltac::Address(ltac::BP, position - 8), static_cast<int>(2 * var->type()->elements()));
            }
        }
    }
    
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

    StatementCompiler compiler(registers, float_registers, target_function, float_pool);

    //Then we compile each of them
    for(auto block : src_function->getBasicBlocks()){
        //If necessary add a label for the block
        if(block_usage.find(block) != block_usage.end()){
            target_function->add(block->label);
        }
    
        //Handle parameters
        compiler.reset();
        compiler.collect_parameters(src_function->definition);
    
        for(unsigned int i = 0; i < block->statements.size(); ++i){
            auto& statement = block->statements[i];

            visit(compiler, statement);
        }

        //end basic block
        if(!compiler.ended){
            compiler.end_basic_block();
        }
    }
    
    //Only if necessary, deallocates size on the stack for the local variables
    if(size > 0){
        ltac::add_instruction(target_function, ltac::Operator::FREE_STACK, size);
    }
        
    ltac::add_instruction(target_function, ltac::Operator::LEAVE);
}
