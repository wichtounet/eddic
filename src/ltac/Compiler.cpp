//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Compiler.hpp"

#include "FunctionContext.hpp"

using namespace eddic;

void ltac::Compiler::compile(std::shared_ptr<mtac::Program> source, std::shared_ptr<ltac::Program> target) const {
    for(auto& src_function : source->functions){
        auto target_function = std::make_shared<ltac::Function>(src_function->context, src_function->getName());

        target->functions.push_back(target_function);

        compile(src_function, target_function);
    }
}

void ltac::Compiler::compile(std::shared_ptr<mtac::Function> src_function, std::shared_ptr<ltac::Function> target_function) const {
    auto size = src_function->context->size();
    
    //Only if necessary, allocates size on the stack for the local variables
    if(size > 0){
        target_function->add(std::make_shared<ltac::Instruction>(ltac::Operator::ALLOC_STACK, size));
    }
    
    auto iter = src_function->context->begin();
    auto end = src_function->context->end();

    for(; iter != end; iter++){
        auto var = iter->second;
        if(var->type().isArray() && var->position().isStack()){
            int position = -var->position().offset();

            target_function->add(std::make_shared<ltac::Instruction>(ltac::Operator::MOV, ltac::Address(ltac::BP, position), var->type().size()));

            if(var->type().base() == BaseType::INT){
                target_function->add(std::make_shared<ltac::Instruction>(ltac::Operator::MEMSET, ltac::Address(ltac::BP, position, -8), var->type().size()));
            } else if(var->type().base() == BaseType::STRING){
                target_function->add(std::make_shared<ltac::Instruction>(ltac::Operator::MEMSET, ltac::Address(ltac::BP, position, -8), 2 * var->type().size()));
            }
        }
    }

    //TODO Does it make sense to have basic blocks ???
    //TODO basic blocks
    
    //Only if necessary, deallocates size on the stack for the local variables
    if(size > 0){
        target_function->add(std::make_shared<ltac::Instruction>(ltac::Operator::FREE_STACK, size));
    }
}
