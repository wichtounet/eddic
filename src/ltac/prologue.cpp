//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "Variable.hpp"

#include "ltac/prologue.hpp"
#include "ltac/Utils.hpp"

using namespace eddic;

void ltac::generate_prologue_epilogue(std::shared_ptr<mtac::Program> ltac_program, std::shared_ptr<Configuration> configuration){
    bool omit_fp = configuration->option_defined("fomit-frame-pointer");
    auto platform = ltac_program->context->target_platform();

    for(auto function : ltac_program->functions){
        auto size = function->context->size();

        //1. Generate prologue
        
        auto bb = function->entry_bb();
    
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

                auto int_size = INT->size(platform);

                if(type->is_array() && type->has_elements()){
                    ltac::add_instruction(bb, ltac::Operator::MOV, ltac::Address(ltac::BP, position), static_cast<int>(type->elements()));
                    ltac::add_instruction(bb, ltac::Operator::MEMSET, ltac::Address(ltac::BP, position + int_size), 
                            static_cast<int>((type->data_type()->size(platform) / int_size * type->elements())));
                } else if(type->is_custom_type()){
                    ltac::add_instruction(bb, ltac::Operator::MEMSET, ltac::Address(ltac::BP, position), static_cast<int>(type->size(platform) / int_size));
                }
            }
        }

        //2. Generate epilogue

        bb = function->exit_bb();

        ltac::add_instruction(bb, ltac::Operator::ADD, ltac::SP, size);

        //Leave stack frame
        if(!omit_fp){
            ltac::add_instruction(bb, ltac::Operator::LEAVE);
        }

        ltac::add_instruction(bb, ltac::Operator::RET);
        
        //3. Generate epilogue for each unresolved RET
        
        for(auto& bb : function){
            auto it = iterate(bb->l_statements);

            while(it.has_next()){
                auto statement = *it;

                if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                    if((*ptr)->op == ltac::Operator::PRE_RET){
                        (*ptr)->op = ltac::Operator::RET;

                        //Leave stack frame
                        if(!omit_fp){
                            it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::LEAVE));
                        }

                        it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::ADD, ltac::SP, size));
                    }
                }

                ++it;
            }
        }
    }
}
