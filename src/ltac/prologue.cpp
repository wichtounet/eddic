//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"

#include "ltac/prologue.hpp"
#include "ltac/Utils.hpp"

using namespace eddic;

ltac::Address stack_address(int offset, bool omit_fp){
    if(omit_fp){
        return ltac::Address(ltac::SP, offset);
    } else {
        return ltac::Address(ltac::BP, offset);
    }
}

void ltac::generate_prologue_epilogue(std::shared_ptr<mtac::Program> mtac_program, std::shared_ptr<Configuration> configuration){
    bool omit_fp = configuration->option_defined("fomit-frame-pointer");
    auto platform = ltac_program->context->target_platform();

    for(auto function : ltac_program->functions){
        auto size = function->context->size();

        //1. Generate prologue
        
        auto bb = function->new_bb_in_front();
    
        //Enter stack frame
        if(!omit_fp){
            ltac::add_instruction(bb, ltac::Operator::ENTER);
        }

        //Allocate stack space for locals
        ltac::add_instruction(bb, ltac::Operator::SUB, ltac::SP, size);

        auto iter = function->context->begin();
        auto end = function->context->end();

        //Clear stack variables
        for(; iter != end; iter++){
            auto var = iter->second;

            //Only stack variables needs to be cleared
            if(var->position().isStack()){
                auto type = var->type();
                int position = var->position().offset();

                if(type->is_array() && type->has_elements()){
                    ltac::add_instruction(bb, ltac::Operator::MOV, stack_address(position, omit_fp), static_cast<int>(type->elements()));
                    ltac::add_instruction(bb, ltac::Operator::MEMSET, stack_address(position + INT->size(platform), omit_fp), static_cast<int>((type->data_type()->size(platform) / INT->size(platform) * type->elements())));
                } else if(type->is_custom_type()){
                    ltac::add_instruction(bb, ltac::Operator::MEMSET, stack_address(position, omit_fp), static_cast<int>(type->size(platform) / INT->size(platform)));
                }
            }
        }

        if(omit_fp){
            //TODO Walk through instructions to modify the offset
            //Add size to each
            //Generate epilogue for each return
        }

        //2. Generate epilogue

        function->new_bb();

        ltac::add_instruction(function, ltac::Operator::ADD, ltac::SP, size);

        //Leave stack frame
        if(!omit_fp){
            ltac::add_instruction(function, ltac::Operator::LEAVE);
        }

        ltac::add_instruction(function, ltac::Operator::RET);
    }
}
