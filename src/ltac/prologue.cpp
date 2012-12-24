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
#include "ltac/Printer.hpp"
#include "ltac/Address.hpp"
#include "ltac/Instruction.hpp"
#include "ltac/Jump.hpp"

using namespace eddic;

namespace {

std::set<ltac::Register> parameter_registers(eddic::Function& function, Platform platform, std::shared_ptr<Configuration> configuration){
    std::set<ltac::Register> overriden_registers;

    auto descriptor = getPlatformDescriptor(platform);

    if(function.standard || configuration->option_defined("fparameter-allocation")){
        unsigned int maxInt = descriptor->numberOfIntParamRegisters();

        for(auto& parameter : function.parameters()){
            auto type = function.getParameterType(parameter.name);
            unsigned int position = function.getParameterPositionByType(parameter.name);

            if(mtac::is_single_int_register(type) && position <= maxInt){
                overriden_registers.insert(ltac::Register(descriptor->int_param_register(position)));
            }
        }
    }

    return overriden_registers;
}

std::set<ltac::FloatRegister> float_parameter_registers(eddic::Function& function, Platform platform, std::shared_ptr<Configuration> configuration){
    std::set<ltac::FloatRegister> overriden_float_registers;
    
    auto descriptor = getPlatformDescriptor(platform);

    if(function.standard || configuration->option_defined("fparameter-allocation")){
        unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();

        for(auto& parameter : function.parameters()){
            auto type = function.getParameterType(parameter.name);
            unsigned int position = function.getParameterPositionByType(parameter.name);

            if(mtac::is_single_float_register(type) && position <= maxFloat){
                overriden_float_registers.insert(ltac::FloatRegister(descriptor->float_param_register(position)));
            }
        }
    }

    return overriden_float_registers;
}

bool callee_save(Function& definition, ltac::Register reg, Platform platform, std::shared_ptr<Configuration> configuration){
    auto return_type = definition.returnType;
    auto descriptor = getPlatformDescriptor(platform);

    //Do not save the return registers
    if((return_type == INT || return_type == BOOL || return_type == CHAR) && reg.reg == descriptor->int_return_register1()){
        return false;
    } else if(return_type == STRING && (reg.reg == descriptor->int_return_register1() || reg.reg == descriptor->int_return_register2())){
        return false;
    } else if(return_type->is_pointer() && reg.reg == descriptor->int_return_register1()){ 
        return false;
    }

    auto parameters = parameter_registers(definition, platform, configuration);

    //Do not save the parameters registers, they are saved by the caller if necessary
    if(parameters.count(reg)){
        return false;
    }

    return true;
}

bool callee_save(eddic::Function& definition, ltac::FloatRegister reg, Platform platform, std::shared_ptr<Configuration> configuration){
    auto return_type = definition.returnType;
    auto descriptor = getPlatformDescriptor(platform);

    //Do not save the return register
    if(return_type == FLOAT && reg.reg == descriptor->float_return_register()){
        return false;
    } 

    auto parameters = float_parameter_registers(definition, platform, configuration);

    //Do not save the parameters registers, they are saved by the caller if necessary
    if(parameters.count(reg)){
        return false;
    }

    return true;
}

void callee_save_registers(mtac::Function& function, mtac::basic_block_p bb, Platform platform, std::shared_ptr<Configuration> configuration){
    //Save registers for all other functions than main
    if(!function.is_main()){
        for(auto& reg : function.use_registers()){
            if(callee_save(function.definition(), reg, platform, configuration)){
                ltac::add_instruction(bb, ltac::Operator::PUSH, reg);
            }
        }

        for(auto& float_reg : function.use_float_registers()){
            if(callee_save(function.definition(), float_reg, platform, configuration)){
                ltac::add_instruction(bb, ltac::Operator::SUB, ltac::SP, static_cast<int>(FLOAT->size(platform)));
                ltac::add_instruction(bb, ltac::Operator::FMOV, ltac::Address(ltac::SP, 0), float_reg);
            }
        }
    }
}

void callee_restore_registers(mtac::Function& function, mtac::basic_block_p bb, Platform platform, std::shared_ptr<Configuration> configuration){
    //Save registers for all other functions than main
    if(!function.is_main()){
        for(auto& float_reg : boost::adaptors::reverse(function.use_float_registers())){
            if(callee_save(function.definition(), float_reg, platform, configuration)){
                ltac::add_instruction(bb, ltac::Operator::FMOV, float_reg, ltac::Address(ltac::SP, 0));
                ltac::add_instruction(bb, ltac::Operator::ADD, ltac::SP, static_cast<int>(FLOAT->size(platform)));
            }
        }

        for(auto& reg : boost::adaptors::reverse(function.use_registers())){
            if(callee_save(function.definition(), reg, platform, configuration)){
                ltac::add_instruction(bb, ltac::Operator::POP, reg);
            }
        }
    }
}

template<typename It>
void callee_restore_registers(mtac::Function& function, It& it, Platform platform, std::shared_ptr<Configuration> configuration){
    //Save registers for all other functions than main
    if(!function.is_main()){
        for(auto& reg : function.use_registers()){
            if(callee_save(function.definition(), reg, platform, configuration)){
                it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::POP, reg));
            }
        }

        for(auto& float_reg : function.use_float_registers()){
            if(callee_save(function.definition(), float_reg, platform, configuration)){
                it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::ADD, ltac::SP, static_cast<int>(FLOAT->size(platform))));
                it.insert(std::make_shared<ltac::Instruction>(ltac::Operator::FMOV, float_reg, ltac::Address(ltac::SP, 0)));
            }
        }
    }
}

template<typename T>
bool contains(const std::set<T>& set, const T& value){
    return set.find(value) != set.end();
}

template<typename T>
bool contains(const std::unordered_set<T>& set, const T& value){
    return set.find(value) != set.end();
}

bool caller_save(mtac::Function& source, eddic::Function& target_definition, ltac::Register reg, Platform platform, std::shared_ptr<Configuration> configuration){
    auto source_parameters = parameter_registers(source.definition(), platform, configuration);
    auto target_parameters = parameter_registers(target_definition, platform, configuration);
    auto variable_registers = source.variable_registers();

    //Only saved is used to hold a variable (not bound) or as a parameter in the source function
    if(contains(target_parameters, reg) && (contains(variable_registers, reg) || contains(source_parameters, reg))){
        return true;
    }

    return false;
}

bool caller_save(mtac::Function& source, eddic::Function& target_definition, ltac::FloatRegister reg, Platform platform, std::shared_ptr<Configuration> configuration){
    auto source_parameters = float_parameter_registers(source.definition(), platform, configuration);
    auto target_parameters = float_parameter_registers(target_definition, platform, configuration);
    auto variable_registers = source.variable_float_registers();

    //Only saved is used to hold a variable (not bound) or as a parameter in the source function
    if(contains(target_parameters, reg) && (contains(variable_registers, reg) || contains(source_parameters, reg))){
        return true;
    }

    return false;
}

template<typename It>
void caller_save_registers(mtac::Function& function, eddic::Function& target_function, mtac::basic_block_p bb, It it, Platform platform, std::shared_ptr<Configuration> configuration){
    auto pre_it = it.it;

    auto call = boost::get<std::shared_ptr<ltac::Jump>>(*it);

    while(true){
        while(pre_it != bb->l_statements.begin()){
            --pre_it;
            
            auto statement = *pre_it;

            if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&statement)){
                if((*ptr)->op == ltac::Operator::PRE_PARAM){
                    (*ptr)->op = ltac::Operator::NOP;

                    for(auto& float_reg : boost::adaptors::reverse(function.use_float_registers())){
                        if(caller_save(function, target_function, float_reg, platform, configuration)){
                            pre_it = bb->l_statements.insert(pre_it, std::make_shared<ltac::Instruction>(ltac::Operator::FMOV, ltac::Address(ltac::SP, 0), float_reg));
                            pre_it = bb->l_statements.insert(pre_it, std::make_shared<ltac::Instruction>(ltac::Operator::SUB, ltac::SP, static_cast<int>(FLOAT->size(platform))));
                        }
                    }
                    
                    for(auto& reg : boost::adaptors::reverse(function.use_registers())){
                        if(caller_save(function, target_function, reg, platform, configuration)){
                            pre_it = bb->l_statements.insert(pre_it, std::make_shared<ltac::Instruction>(ltac::Operator::PUSH, reg));
                        }
                    }

                    return;
                }
            }
        }

        bb = bb->prev;
        pre_it = bb->l_statements.end();
    }
}

template<typename It, typename Type>
void find(It& it, const Type& value){
    while(true){
        if(auto* ptr = boost::get<Type>(&*it)){
            if(*ptr == value){
                return;
            }
        }

        ++it;
    }
}

template<typename It>
void caller_cleanup(mtac::Function& function, eddic::Function& target_function, mtac::basic_block_p bb, It it, Platform platform, std::shared_ptr<Configuration> configuration){
    auto call = boost::get<std::shared_ptr<ltac::Jump>>(*it);

    caller_save_registers(function, target_function, bb, it, platform, configuration);

    //The iterator has been invalidated by the save, find the call again
    it.restart();
    find(it, call);

    if(it.has_next()){
        ++it;

        bool rewind = true;

        if(auto* ptr = boost::get<std::shared_ptr<ltac::Instruction>>(&*it)){
            if((*ptr)->op == ltac::Operator::ADD){
                if(auto* reg_ptr = boost::get<ltac::Register>(&*(*ptr)->arg1)){
                    if(*reg_ptr == ltac::SP){
                       rewind = false;
                    }
                }
            }
        }

        if(rewind){
            --it;
        }
    }

    for(auto& float_reg : boost::adaptors::reverse(function.use_float_registers())){
        if(caller_save(function, target_function, float_reg, platform, configuration)){
            it.insert_after(std::make_shared<ltac::Instruction>(ltac::Operator::FMOV, float_reg, ltac::Address(ltac::SP, 0)));
            it.insert_after(std::make_shared<ltac::Instruction>(ltac::Operator::ADD, ltac::SP, static_cast<int>(FLOAT->size(platform))));
        }
    }
    
    for(auto& reg : boost::adaptors::reverse(function.use_registers())){
        if(caller_save(function, target_function, reg, platform, configuration)){
            it.insert_after(std::make_shared<ltac::Instruction>(ltac::Operator::POP, reg));
        }
    }
}

} //End of anonymous

void ltac::generate_prologue_epilogue(mtac::Program& program, std::shared_ptr<Configuration> configuration){
    bool omit_fp = configuration->option_defined("fomit-frame-pointer");
    auto platform = program.context->target_platform();

    for(auto& function : program.functions){
        auto size = function.context->size();

        //1. Generate prologue
        
        auto bb = function.entry_bb();
    
        //Enter stack frame
        if(!omit_fp){
            ltac::add_instruction(bb, ltac::Operator::ENTER);
        }

        //Allocate stack space for locals
        ltac::add_instruction(bb, ltac::Operator::SUB, ltac::SP, size);

        auto iter = function.context->begin();
        auto end = function.context->end();

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

                    //Set lengths of arrays inside structures
                    auto struct_type = function.context->global()->get_struct(type);
                    auto offset = 0;

                    while(struct_type){
                        for(auto& member : struct_type->members){
                            if(member->type->is_array() && !member->type->is_dynamic_array()){
                                ltac::add_instruction(bb, ltac::Operator::MOV, 
                                        ltac::Address(ltac::BP, position + offset + function.context->global()->member_offset(struct_type, member->name)),
                                        static_cast<int>(member->type->elements()));
                            }
                        }

                        struct_type = function.context->global()->get_struct(struct_type->parent_type);
                    }
                }
            }
        }

        callee_save_registers(function, bb, platform, configuration);

        //2. Generate epilogue

        bb = function.exit_bb();

        callee_restore_registers(function, bb, platform, configuration);

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

                        callee_restore_registers(function, it, platform, configuration);
                    }
                }

                ++it;
            }
        }

        //4. Generate caller save/restore code
       
        for(auto& bb : function){
            auto it = iterate(bb->l_statements);

            while(it.has_next()){
                auto statement = *it;

                if(auto* ptr = boost::get<std::shared_ptr<ltac::Jump>>(&statement)){
                    if((*ptr)->type == ltac::JumpType::CALL){
                        caller_cleanup(function, *(*ptr)->target_function, bb, it, platform, configuration);

                        //The iterator is invalidated by the cleanup, necessary to find the call again
                        it.restart();
                        find(it, *ptr);
                    }
                }

                ++it;
            }
        }
    }
}
