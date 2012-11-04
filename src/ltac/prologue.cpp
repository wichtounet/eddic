//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/range/adaptors.hpp>

#include "GlobalContext.hpp"
#include "FunctionContext.hpp"
#include "Type.hpp"
#include "Variable.hpp"

#include "ltac/prologue.hpp"
#include "ltac/Utils.hpp"

using namespace eddic;

namespace {

bool callee_save(mtac::function_p function, ltac::Register reg, Platform platform){
    auto definition = function->definition; 
    auto return_type = definition->returnType;
    auto descriptor = getPlatformDescriptor(platform);

    if((return_type == INT || return_type == BOOL || return_type == CHAR) && reg.reg == descriptor->int_return_register1()){
        return false;
    } else if(return_type == STRING && (reg.reg == descriptor->int_return_register1() || reg.reg == descriptor->int_return_register2())){
        return false;
    } else if(return_type->is_pointer() && reg.reg == descriptor->int_return_register1()){ 
        return false;
    }

    return true;
}

void save_registers(mtac::function_p function, mtac::basic_block_p bb, Platform platform){
    //Save registers for all other functions than main
    if(!function->is_main()){
        //TODO Ignore return register (if used as return), parameter register (if used as parameter)

        for(auto& reg : function->use_registers()){
            if(callee_save(function, reg, platform)){
                ltac::add_instruction(bb, ltac::Operator::PUSH, reg);
            }
        }

        for(auto& float_reg : function->use_float_registers()){
            ltac::add_instruction(bb, ltac::Operator::SUB, ltac::SP, static_cast<int>(FLOAT->size(platform)));
            ltac::add_instruction(bb, ltac::Operator::FMOV, ltac::Address(ltac::SP, 0), float_reg);
        }
    }
}

void restore_registers(mtac::function_p function, mtac::basic_block_p bb, Platform platform){
    //Save registers for all other functions than main
    if(!function->is_main()){
        //TODO Ignore return register (if used as return), parameter register (if used as parameter)

        for(auto& float_reg : boost::adaptors::reverse(function->use_float_registers())){
            ltac::add_instruction(bb, ltac::Operator::FMOV, float_reg, ltac::Address(ltac::SP, 0));
            ltac::add_instruction(bb, ltac::Operator::ADD, ltac::SP, static_cast<int>(FLOAT->size(platform)));
        }

        for(auto& reg : boost::adaptors::reverse(function->use_registers())){
            if(callee_save(function, reg, platform)){
                ltac::add_instruction(bb, ltac::Operator::POP, reg);
            }
        }
    }
}

template<typename It>
void restore_registers(mtac::function_p function, It& it, Platform platform){
    //Save registers for all other functions than main
    if(!function->is_main()){
        //TODO Ignore return register (if used as return), parameter register (if used as parameter)

        for(auto& reg : function->use_registers()){
            if(callee_save(function, reg, platform)){
                it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::POP, reg));
            }
        }

        for(auto& float_reg : function->use_float_registers()){
            it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::ADD, ltac::SP, static_cast<int>(FLOAT->size(platform))));
            it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::FMOV, float_reg, ltac::Address(ltac::SP, 0)));
        }
    }
}

} //End of anonymous

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

        save_registers(function, bb, platform);

        //2. Generate epilogue

        bb = function->exit_bb();

        restore_registers(function, bb, platform);

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

                        restore_registers(function, it, platform);
                    }
                }

                ++it;
            }
        }
    }
}
