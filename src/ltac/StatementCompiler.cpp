//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "FunctionContext.hpp"
#include "Utils.hpp"
#include "Type.hpp"
#include "Options.hpp"
#include "Labels.hpp"
#include "Variable.hpp"
#include "logging.hpp"
#include "Function.hpp"

#include "mtac/Quadruple.hpp"
#include "mtac/Argument.hpp"
#include "mtac/Utils.hpp" 

#include "ltac/StatementCompiler.hpp"
#include "ltac/Utils.hpp"
#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"
#include "ltac/PseudoRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"

using namespace eddic;

namespace {

ltac::Address stack_address(int offset){
    return ltac::Address(ltac::BP, offset);
}

ltac::Address stack_address(ltac::AddressRegister offsetReg, int offset){
    return ltac::Address(ltac::BP, offsetReg, 1, offset);
}

ltac::Size convert_size(mtac::Size size){
    switch(size){
        case mtac::Size::BYTE:
            return ltac::Size::BYTE;
        case mtac::Size::WORD:
            return ltac::Size::WORD;
        case mtac::Size::DOUBLE_WORD:
            return ltac::Size::DOUBLE_WORD;
        case mtac::Size::QUAD_WORD:
            return ltac::Size::QUAD_WORD;
        case mtac::Size::DEFAULT:
            return ltac::Size::DEFAULT;
    }
}

} //end of anonymous namespace

ltac::StatementCompiler::StatementCompiler(std::shared_ptr<FloatPool> float_pool) : manager(float_pool), float_pool(float_pool) {}

void ltac::StatementCompiler::end_bb(){
    if(ended){
        return;
    }

    for(auto& var : manager.local){
        if(manager.is_written(var)){
            auto position = var->position();

            if(var->type() == FLOAT){
                auto reg = manager.get_pseudo_float_reg(var);

                if(position.isStack() || position.isParameter()){
                    bb->emplace_back_low(ltac::Operator::FMOV, ltac::Address(ltac::BP, position.offset()), reg);
                } else if(position.isGlobal()){
                    bb->emplace_back_low(ltac::Operator::FMOV, ltac::Address("V" + position.name()), reg);
                } else {
                    eddic_unreachable("Invalid position");
                }
            } else {
                auto reg = manager.get_pseudo_reg(var);

                if(position.isStack() || position.isParameter()){
                    bb->emplace_back_low(ltac::Operator::MOV, ltac::Address(ltac::BP, position.offset()), reg);
                } else if(position.isGlobal()){
                    bb->emplace_back_low(ltac::Operator::MOV, ltac::Address("V" + position.name()), reg);
                } else {
                    eddic_unreachable("Invalid position");
                }
            }
        }

        if(var->type() == FLOAT){
            manager.remove_from_pseudo_float_reg(var);
        } else {
            manager.remove_from_pseudo_reg(var);
        }
    }

    manager.reset();

    ended = true;
}

void ltac::StatementCompiler::collect_parameters(eddic::Function& definition){
    manager.collect_parameters(definition, descriptor);
}

ltac::PseudoRegister ltac::StatementCompiler::to_register(std::shared_ptr<Variable> var){
    return ltac::to_register(var, manager);
}

ltac::Argument ltac::StatementCompiler::to_arg(mtac::Argument argument){
    return ltac::to_arg(argument, manager);
}

ltac::Address ltac::StatementCompiler::address(std::shared_ptr<Variable> var, mtac::Argument offset){
    if(var->type()->is_pointer() || (var->type()->is_dynamic_array() && !var->position().isParameter())){
        auto reg = manager.get_pseudo_reg(var);

        if(auto* ptr = boost::get<int>(&offset)){
            return ltac::Address(reg, *ptr);
        } else {
            auto offsetReg = manager.get_pseudo_reg(ltac::get_variable(offset));
            return ltac::Address(reg, offsetReg);
        }
    } else {
        auto position = var->position();

        if(auto* ptr = boost::get<int>(&offset)){
            if(position.isStack()){
                return stack_address(position.offset() + *ptr);
            } else if(position.isParameter()){
                //The case of array is special because only the address is passed, not the complete array
                if(var->type()->is_array()){
                    auto reg = manager.get_free_pseudo_reg();

                    bb->emplace_back_low(ltac::Operator::MOV, reg, stack_address(position.offset()));

                    return ltac::Address(reg, *ptr);
                //In the other cases, the value is passed, so we can compute the offset directly
                } else {
                    return stack_address(position.offset() + *ptr);
                }
            } else if(position.isGlobal()){
                return ltac::Address("V" + position.name(), *ptr);
            } 

            auto reg = manager.get_pseudo_reg(var);
            return ltac::Address(reg, *ptr);
        }

        auto offsetReg = manager.get_pseudo_reg(ltac::get_variable(offset));

        if(position.isStack()){
            return stack_address(offsetReg, position.offset());
        } else if(position.isParameter()){
            auto reg = manager.get_free_pseudo_reg();

            //The case of array is special because only the address is passed, not the complete array
            if(var->type()->is_array()){
                bb->emplace_back_low(ltac::Operator::MOV, reg, stack_address(position.offset()));
            } else {
                bb->emplace_back_low(ltac::Operator::LEA, reg, stack_address(position.offset()));
            }

            return ltac::Address(reg, offsetReg);
        } else if(position.isGlobal()){
            return ltac::Address("V" + position.name(), offsetReg);
        } 

        assert(position.is_temporary());

        auto reg = manager.get_pseudo_reg(var);
        return ltac::Address(reg, offsetReg);
    }
}

void ltac::StatementCompiler::pass_in_int_register(mtac::Argument& argument, int position){
    auto reg = manager.get_bound_pseudo_reg(descriptor->int_param_register(position));
    bb->emplace_back_low(ltac::Operator::MOV, reg, to_arg(argument));
    uses.push_back(reg);
}

void ltac::StatementCompiler::pass_in_float_register(mtac::Argument& argument, int position){
    auto reg = manager.get_bound_pseudo_float_reg(descriptor->float_param_register(position));

    if(auto* ptr = boost::get<int>(&argument)){
        auto label = float_pool->label(static_cast<double>(*ptr));
        bb->emplace_back_low(ltac::Operator::FMOV, reg, ltac::Address(label));
    } else if(auto* ptr = boost::get<double>(&argument)){
        auto label = float_pool->label(*ptr);
        bb->emplace_back_low(ltac::Operator::FMOV, reg, ltac::Address(label));
    } else {
        bb->emplace_back_low(ltac::Operator::FMOV, reg, to_arg(argument));
    }
    
    float_uses.push_back(reg);
}

void ltac::StatementCompiler::compare_binary(mtac::Argument& arg1, mtac::Argument& arg2){
    //The first argument is not important, it can be immediate, but the second must be a register
    if(auto* ptr = boost::get<int>(&arg1)){
        auto reg1 = manager.get_free_pseudo_reg();

        bb->emplace_back_low(ltac::Operator::MOV, reg1, *ptr);

        auto reg2 = manager.get_pseudo_reg(ltac::get_variable(arg2));
        
        end_bb();

        bb->emplace_back_low(ltac::Operator::CMP_INT, reg1, reg2);
    } else {
        auto reg1 = manager.get_pseudo_reg(ltac::get_variable(arg1));
        auto reg2 = to_arg(arg2);
        
        end_bb();

        bb->emplace_back_low(ltac::Operator::CMP_INT, reg1, reg2);
    }
}

void ltac::StatementCompiler::compare_float_binary(mtac::Argument& arg1, mtac::Argument& arg2){
    //Comparisons of constant should have been handled by the optimizer
    assert(!(mtac::isFloat(arg1) && mtac::isFloat(arg2))); 

    //If both args are variables
    if(mtac::isVariable(arg1) && mtac::isVariable(arg2)){
        auto reg1 = manager.get_pseudo_float_reg(ltac::get_variable(arg1));
        auto reg2 = manager.get_pseudo_float_reg(ltac::get_variable(arg2));
        
        end_bb();

        bb->emplace_back_low(ltac::Operator::CMP_FLOAT, reg1, reg2);
    } else if(mtac::isVariable(arg1) && mtac::isFloat(arg2)){
        auto reg1 = manager.get_pseudo_float_reg(ltac::get_variable(arg1));
        auto reg2 = manager.get_free_pseudo_float_reg();

        manager.copy(arg2, reg2);
        
        end_bb();

        bb->emplace_back_low(ltac::Operator::CMP_FLOAT, reg1, reg2);
    } else if(mtac::isFloat(arg1) && mtac::isVariable(arg2)){
        auto reg1 = manager.get_free_pseudo_float_reg();
        auto reg2 = manager.get_pseudo_float_reg(ltac::get_variable(arg2));

        manager.copy(arg1, reg1);
        
        end_bb();

        bb->emplace_back_low(ltac::Operator::CMP_FLOAT, reg1, reg2);
    }
}

void ltac::StatementCompiler::compare_unary(mtac::Argument arg1){
    if(auto* ptr = boost::get<int>(&arg1)){
        auto reg = manager.get_free_pseudo_reg();

        bb->emplace_back_low(ltac::Operator::MOV, reg, *ptr);

        end_bb();

        bb->emplace_back_low(ltac::Operator::OR, reg, reg);
    } else {
        auto reg = manager.get_pseudo_reg(ltac::get_variable(arg1));
        
        end_bb();

        bb->emplace_back_low(ltac::Operator::OR, reg, reg);
    }
}

void ltac::StatementCompiler::set_if_cc(ltac::Operator set, mtac::Quadruple& quadruple, bool floats){
    auto reg = manager.get_pseudo_reg_no_move(quadruple.result);

    //The default value is 0
    bb->emplace_back_low(ltac::Operator::MOV, reg, 0);

    //The first argument is not important, it can be immediate, but the second must be a register
    if(auto* ptr = boost::get<int>(&*quadruple.arg1)){
        auto cmp_reg = manager.get_free_pseudo_reg();

        bb->emplace_back_low(ltac::Operator::MOV, cmp_reg, *ptr); 
        
        if(floats){
            bb->emplace_back_low(ltac::Operator::CMP_FLOAT, cmp_reg, to_arg(*quadruple.arg2)); 
        } else {
            bb->emplace_back_low(ltac::Operator::CMP_INT, cmp_reg, to_arg(*quadruple.arg2)); 
        }
    } 
    //For ucomisd, neither the first nor the second argument can be a double
    else if(auto* ptr = boost::get<double>(&*quadruple.arg1)){
        auto cmp_reg = manager.get_free_pseudo_float_reg();

        auto label = float_pool->label(*ptr);
        bb->emplace_back_low(ltac::Operator::FMOV, cmp_reg, ltac::Address(label)); 
        
        if(floats){
            bb->emplace_back_low(ltac::Operator::CMP_FLOAT, cmp_reg, to_arg(*quadruple.arg2)); 
        } else {
            bb->emplace_back_low(ltac::Operator::CMP_INT, cmp_reg, to_arg(*quadruple.arg2)); 
        }
    }
    //For ucomisd, neither the first nor the second argument can be a double
    else if(auto* ptr = boost::get<double>(&*quadruple.arg2)){
        auto cmp_reg = manager.get_free_pseudo_float_reg();

        auto label = float_pool->label(*ptr);
        bb->emplace_back_low(ltac::Operator::FMOV, cmp_reg, ltac::Address(label)); 
        
        if(floats){
            bb->emplace_back_low(ltac::Operator::CMP_FLOAT, to_arg(*quadruple.arg1), cmp_reg); 
        } else {
            bb->emplace_back_low(ltac::Operator::CMP_INT, to_arg(*quadruple.arg1), cmp_reg); 
        }
    } else {
        if(floats){
            bb->emplace_back_low(ltac::Operator::CMP_FLOAT, to_arg(*quadruple.arg1), to_arg(*quadruple.arg2)); 
        } else {
            bb->emplace_back_low(ltac::Operator::CMP_INT, to_arg(*quadruple.arg1), to_arg(*quadruple.arg2)); 
        }
    }

    //Conditionally move 1 in the register
    auto value_reg = manager.get_free_pseudo_reg();
    bb->emplace_back_low(ltac::Operator::MOV, value_reg, 1); 
    bb->emplace_back_low(set, reg, value_reg); 

    manager.set_written(quadruple.result);
}
        
void ltac::StatementCompiler::push(ltac::Argument arg){
    bb->emplace_back_low(ltac::Operator::PUSH, arg);
}

void ltac::StatementCompiler::pop(ltac::Argument arg){
    bb->emplace_back_low(ltac::Operator::POP, arg);
}

void ltac::StatementCompiler::compile_GOTO(mtac::Quadruple& quadruple){
    LOG<Trace>("Registers") << "Current statement " << quadruple << log::endl;

    end_bb();

    bb->emplace_back_low(quadruple.block->label, ltac::Operator::ALWAYS);
}

ltac::PseudoRegister ltac::StatementCompiler::get_address_in_pseudo_reg(std::shared_ptr<Variable> var, int offset){
    auto reg = manager.get_free_pseudo_reg();

    bb->emplace_back_low(ltac::Operator::LEA, reg, address(var, offset));
    
    return reg;
}

ltac::PseudoRegister ltac::StatementCompiler::get_address_in_pseudo_reg2(std::shared_ptr<Variable> var, ltac::PseudoRegister offset){
    auto reg = manager.get_free_pseudo_reg();
    
    bb->emplace_back_low(ltac::Operator::LEA, reg, address(var, 0));
    bb->emplace_back_low(ltac::Operator::ADD, reg, offset);
    
    return reg;
}

std::tuple<std::shared_ptr<const Type>, bool, unsigned int> ltac::StatementCompiler::common_param(mtac::Quadruple& param){
    LOG<Trace>("Registers") << "Current statement " << param << log::endl;

    if(first_param){
        bb->emplace_back_low(ltac::Operator::PRE_PARAM);

        //Align stack pointer to the size of an INT

        auto total = function_stack_size(param.function());
        if(total % INT->size(platform) != 0){
            int padding = INT->size(platform) - (total % INT->size(platform));

            bb->emplace_back_low(ltac::Operator::SUB, ltac::SP, padding);
        }

        first_param = false;
    }
    
    std::shared_ptr<const Type> type;
    bool register_allocated = false;
    unsigned int position = 0;
        
    if(param.std_param().length() > 0 || (param.param() && configuration->option_defined("fparameter-allocation"))){
        unsigned int maxInt = descriptor->numberOfIntParamRegisters();
        unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();

        //It's a call to a standard function
        if(param.std_param().length() > 0){
            type = param.function().parameter(param.std_param()).type();
            position = param.function().parameter_position_by_type(param.std_param());
        } 
        //It's a call to a user function
        else if(param.param()){
            type = param.param()->type();
            position = param.function().parameter_position_by_type(param.param()->name());
        }

        register_allocated = 
                (mtac::is_single_int_register(type) && position <= maxInt)
            ||  (mtac::is_single_float_register(type) && position <= maxFloat);
    }

    return std::make_tuple(type, register_allocated, position);
}

void ltac::StatementCompiler::compile_PARAM(mtac::Quadruple& param){
    std::shared_ptr<const Type> type;
    bool register_allocated;
    unsigned int position;

    std::tie(type, register_allocated, position) = common_param(param);

    //1. If register allocated, find the correct register and move the value into it

    if(register_allocated){
        if(auto* ptr = boost::get<int>(&*param.arg1)){
            if(*ptr == 0){
                if(type == FLOAT){
                    pass_in_float_register(*param.arg1, position);
                    return;
                } 
            } 
        }

        if(mtac::is_single_int_register(type)){
            pass_in_int_register(*param.arg1, position);
        } else {
            pass_in_float_register(*param.arg1, position);
        }

        return;
    }
    
    //2. If the param as not been handled as register passing, push it on the stack 

    //Char has a smaller size, cannot use push instructions
    
    if(param.param() && (param.param()->type() == CHAR || param.param()->type() == BOOL)){
        bb->emplace_back_low(ltac::Operator::SUB, ltac::SP, 1);

        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*param.arg1)){
            auto& var = *ptr;
            auto reg = manager.get_pseudo_reg(var);
                
            //Necessary to obtain an hard reg here to be sure that it 8-bit allocatable
            auto hard_reg = manager.get_bound_pseudo_reg(descriptor->d_register());
            bb->emplace_back_low(ltac::Operator::MOV, hard_reg, reg);

            ltac::Instruction mov(ltac::Operator::MOV, ltac::Address(ltac::SP, 0), hard_reg);
            mov.size = ltac::Size::BYTE;
            bb->push_back(std::move(mov));

            uses.push_back(hard_reg);
        } else {
            //If it is not a variable it can only be an int (char value)
            auto value = boost::get<int>(*param.arg1);
            
            ltac::Instruction mov(ltac::Operator::MOV, ltac::Address(ltac::SP, 0), value);
            mov.size = ltac::Size::BYTE;
            bb->push_back(std::move(mov));
        }

        return;
    }

    //Use push instructions for regular types

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*param.arg1)){
        auto var = *ptr;

        if(!var->type()->is_array() && ltac::is_float_var(var)){
            auto reg1 = manager.get_free_pseudo_reg();
            auto reg2 = manager.get_pseudo_float_reg(var);

            bb->emplace_back_low(ltac::Operator::MOV, reg1, reg2);
            push(reg1);
        } else {
            if(var->type()->is_array() && !var->type()->is_dynamic_array()){
                auto position = (*ptr)->position();

                if(position.isGlobal()){
                    auto reg = manager.get_free_pseudo_reg();

                    bb->emplace_back_low(ltac::Operator::MOV, reg, "V" + position.name());
                    push(reg);
                } else if(position.isStack()){
                    auto reg = manager.get_free_pseudo_reg();

                    bb->emplace_back_low(ltac::Operator::LEA, reg, stack_address(position.offset()));
                    push(reg);
                } else if(position.isParameter()){
                    push(stack_address(position.offset()));
                }
            } else if(var->type() == CHAR || var->type() == BOOL){
                auto reg = manager.get_pseudo_reg(var);

                //Necessary to obtain an hard reg here to be sure that it 8-bit allocatable
                auto hard_reg = manager.get_bound_pseudo_reg(descriptor->d_register());
                bb->emplace_back_low(ltac::Operator::MOV, hard_reg, reg);

                bb->emplace_back_low(ltac::Operator::SUB, ltac::SP, 1);

                ltac::Instruction mov(ltac::Operator::MOV, ltac::Address(ltac::SP, 0), hard_reg);
                mov.size = ltac::Size::BYTE;
                bb->push_back(std::move(mov));

                uses.push_back(hard_reg);
            } else {
                auto reg = manager.get_pseudo_reg(var);
                push(reg);
            }
        }
    } else if(auto* ptr = boost::get<int>(&*param.arg1)){
        if(*ptr == 0){
            if(param.param() && param.param()->type() == FLOAT){
                auto label = float_pool->label(0.0);
                push(ltac::Address(label));
            } else if(!param.std_param().empty() && param.function().parameter(param.std_param()).type() == FLOAT){
                auto label = float_pool->label(0.0);
                push(ltac::Address(label));
            } else {
                push(to_arg(*param.arg1));
            }
        } else {
            push(to_arg(*param.arg1));
        }
    } else if(auto* ptr = boost::get<double>(&*param.arg1)){
        auto label = float_pool->label(*ptr);
        push(ltac::Address(label));
    } else {
        push(to_arg(*param.arg1));
    }
}

void ltac::StatementCompiler::compile_PPARAM(mtac::Quadruple& param){
    std::shared_ptr<const Type> type;
    bool register_allocated;
    unsigned int position;

    std::tie(type, register_allocated, position) = common_param(param);

    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*param.arg1)){
        auto variable = *ptr;

        if(variable->type()->is_pointer()){
            auto reg = manager.get_pseudo_reg(variable);

            if(register_allocated){
                auto param_reg = manager.get_bound_pseudo_reg(descriptor->int_param_register(position));
                bb->emplace_back_low(ltac::Operator::MOV, param_reg, reg);
                uses.push_back(param_reg);
            } else {
                push(reg);
            }
        } else {
            auto reg = get_address_in_pseudo_reg(variable, 0);

            if(register_allocated){
                auto param_reg = manager.get_bound_pseudo_reg(descriptor->int_param_register(position));
                bb->emplace_back_low(ltac::Operator::MOV, param_reg, reg);
                uses.push_back(param_reg);
            } else {
                push(reg);
            }
        }
    } else {
        auto value = boost::get<int>(*param.arg1);
        push(value);
    }
}

int ltac::StatementCompiler::function_stack_size(eddic::Function& function){
    int total = 0;

    unsigned int maxInt = descriptor->numberOfIntParamRegisters();
    unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();
    
    if(!function.standard() && !configuration->option_defined("fparameter-allocation")){
        maxInt = 0;
        maxFloat = 0;
    }

    for(auto& param : function.parameters()){
        auto type = param.type(); 

        if(type->is_array()){
            //Passing an array is just passing an address
            total += INT->size(platform);
        } else {
            if(mtac::is_single_int_register(type)){
                //If the parameter is allocated in a register, there is no need to deallocate stack space for it
                if(maxInt > 0){
                    --maxInt;
                } else {
                    total += type->size(platform);
                }
            } else if(mtac::is_single_float_register(type)){
                //If the parameter is allocated in a register, there is no need to deallocate stack space for it
                if(maxFloat > 0){
                    --maxFloat;
                } else {
                    total += type->size(platform);
                }
            } else {
                total += type->size(platform);
            }
        }
    }

    return total;
}

void ltac::StatementCompiler::compile_CALL(mtac::Quadruple& call){
    LOG<Trace>("Registers") << "Current statement " << call << log::endl;

    //Means that there are no params
    if(first_param){
        bb->emplace_back_low(ltac::Operator::PRE_PARAM);
    }

    first_param = true;

    //Compute the size of the parameters
    auto total = function_stack_size(call.function());

    //Align stack pointer to the size of an INT

    if(total % INT->size(platform) != 0){
        int padding = INT->size(platform) - (total % INT->size(platform));
        total += padding;
    }

    ltac::Instruction call_instruction(call.function().mangled_name(), ltac::Operator::CALL);
    call_instruction.target_function = &call.function();
    call_instruction.uses = uses;
    call_instruction.float_uses = float_uses;

    ltac::PseudoRegister return_reg_1;
    ltac::PseudoRegister return_reg_2;
    ltac::PseudoFloatRegister return_float_reg_1;
    
    if(call.return1()){
        if(call.return1()->type() == FLOAT){
            return_float_reg_1 = manager.get_bound_pseudo_float_reg(descriptor->float_return_register());
            call_instruction.float_kills.push_back(return_float_reg_1);
        } else {
            return_reg_1 = manager.get_bound_pseudo_reg(descriptor->int_return_register1());
            call_instruction.kills.push_back(return_reg_1);
        }
    }

    if(call.return2()){
        return_reg_2 = manager.get_bound_pseudo_reg(descriptor->int_return_register2());
        call_instruction.kills.push_back(return_reg_2);
    }
    
    bb->l_statements.push_back(std::move(call_instruction));

    uses.clear();
    float_uses.clear();

    //Deallocate space of the parameters

    bb->emplace_back_low(ltac::Operator::ADD, ltac::SP, total);

    //The copies should be cleaned by the optimizations

    if(call.return1()){
        if(call.return1()->type() == FLOAT){
            auto reg = manager.get_pseudo_float_reg_no_move(call.return1());
            bb->emplace_back_low(ltac::Operator::FMOV, reg, return_float_reg_1);
        } else {
            auto reg = manager.get_pseudo_reg_no_move(call.return1());
            bb->emplace_back_low(ltac::Operator::MOV, reg, return_reg_1);
        }

        manager.set_written(call.return1());
    }

    if(call.return2()){
        auto reg = manager.get_pseudo_reg_no_move(call.return2());
        bb->emplace_back_low(ltac::Operator::MOV, reg, return_reg_2);

        manager.set_written(call.return2());
    }
}

void ltac::StatementCompiler::compile_ASSIGN(mtac::Quadruple& quadruple){
    auto reg = manager.get_pseudo_reg_no_move(quadruple.result);
    
    //Copy it in the register
    manager.copy(*quadruple.arg1, reg, convert_size(quadruple.size));

    //The variable has been written
    manager.set_written(quadruple.result);

    //If the address of the variable is escaped, we have to spill its value directly
    if(manager.is_escaped(quadruple.result)){
        auto position = quadruple.result->position();
            
        //TODO if the reg is already 8-bit allocatable, use it directly

        //Necessary to obtain an hard reg here to be sure that it 8-bit allocatable
        auto hard_reg = manager.get_bound_pseudo_reg(descriptor->d_register());

        bb->emplace_back_low(ltac::Operator::MOV, hard_reg, reg);

        if(position.isStack()){
            ltac::Instruction mov(ltac::Operator::MOV, ltac::Address(ltac::BP, position.offset()), hard_reg);
            mov.size = convert_size(quadruple.size);
            bb->push_back(std::move(mov));
        } else if(position.isGlobal()){
            ltac::Instruction mov(ltac::Operator::MOV, ltac::Address("V" + position.name()), hard_reg);
            mov.size = convert_size(quadruple.size);
            bb->push_back(std::move(mov));
        } else {
            eddic_unreachable("Invalid position");
        }
        
        uses.push_back(hard_reg);
        
        manager.remove_from_pseudo_reg(quadruple.result); 
    }
}

void ltac::StatementCompiler::compile_PASSIGN(mtac::Quadruple& quadruple){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
        if((*ptr)->type()->is_pointer()){
            compile_ASSIGN(quadruple);
        } else {
            auto result_reg = manager.get_pseudo_reg_no_move(quadruple.result);
            auto value_reg = get_address_in_pseudo_reg(*ptr, 0);
            bb->emplace_back_low(ltac::Operator::MOV, result_reg, value_reg);

            manager.set_written(quadruple.result);

            //If the address of the variable is escaped, we have to spill its value directly
            if(manager.is_escaped(quadruple.result)){
                auto position = quadruple.result->position();
                if(position.isStack()){
                    bb->emplace_back_low(ltac::Operator::MOV, ltac::Address(ltac::BP, position.offset()), value_reg);
                } else if(position.isGlobal()){
                    bb->emplace_back_low(ltac::Operator::MOV, ltac::Address("V" + position.name()), value_reg);
                } else {
                    eddic_unreachable("Invalid position");
                }

                manager.remove_from_pseudo_reg(quadruple.result); 
            }
        }
    } else {
        compile_ASSIGN(quadruple);
    }
}

void ltac::StatementCompiler::compile_FASSIGN(mtac::Quadruple& quadruple){
    auto reg = manager.get_pseudo_float_reg_no_move(quadruple.result);
    manager.copy(*quadruple.arg1, reg);

    manager.set_written(quadruple.result);

    //If the address of the variable is escaped, we have to spill its value directly
    if(manager.is_escaped(quadruple.result)){
        auto position = quadruple.result->position();
        if(position.isStack()){
            bb->emplace_back_low(ltac::Operator::FMOV, ltac::Address(ltac::BP, position.offset()), reg);
        } else if(position.isGlobal()){
            bb->emplace_back_low(ltac::Operator::FMOV, ltac::Address("V" + position.name()), reg);
        } else {
            eddic_unreachable("Invalid position");
        }
        
        manager.remove_from_pseudo_float_reg(quadruple.result); 
    }
}

void ltac::StatementCompiler::compile_ADD(mtac::Quadruple& quadruple){
    auto result = quadruple.result;

    //Optimize the special form a = a + b by using only one ADD instruction
    if(*quadruple.arg1 == result){
        auto reg = manager.get_pseudo_reg(quadruple.result);
        bb->emplace_back_low(ltac::Operator::ADD, reg, to_arg(*quadruple.arg2));
    } 
    //Optimize the special form a = b + a by using only one ADD instruction
    else if(*quadruple.arg2 == result){
        auto reg = manager.get_pseudo_reg(quadruple.result);
        bb->emplace_back_low(ltac::Operator::ADD, reg, to_arg(*quadruple.arg2));
    } 
    //In the other cases, use lea to perform the addition
    else {
        auto reg = manager.get_pseudo_reg_no_move(quadruple.result);

        if(ltac::is_variable(*quadruple.arg1)){
            if(ltac::is_variable(*quadruple.arg2)){
                bb->emplace_back_low(ltac::Operator::LEA, reg, ltac::Address(
                            manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg1)), 
                            manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg2))));
            } else {
                bb->emplace_back_low(ltac::Operator::LEA, reg, ltac::Address(
                            manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg1)), boost::get<int>(*quadruple.arg2)));
            }
        } else {
            if(ltac::is_variable(*quadruple.arg2)){
                bb->emplace_back_low(ltac::Operator::LEA, reg, 
                        ltac::Address(manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg2)), boost::get<int>(*quadruple.arg1)));
            } else {
                bb->emplace_back_low(ltac::Operator::MOV, reg, boost::get<int>(*quadruple.arg1) + boost::get<int>(*quadruple.arg2));
            }
        }
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_SUB(mtac::Quadruple& quadruple){
    auto result = quadruple.result;

    //Optimize the special form a = a - b by using only one SUB instruction
    if(*quadruple.arg1 == result){
        auto reg = manager.get_pseudo_reg(quadruple.result);
        bb->emplace_back_low(ltac::Operator::SUB, reg, to_arg(*quadruple.arg2));
    } 
    //In the other cases, move the first arg into the result register and then subtract the second arg into it
    else {
        auto reg = manager.get_pseudo_reg_no_move(quadruple.result);
        bb->emplace_back_low(ltac::Operator::MOV, reg, to_arg(*quadruple.arg1));
        bb->emplace_back_low(ltac::Operator::SUB, reg, to_arg(*quadruple.arg2));
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_MUL(mtac::Quadruple& quadruple){
    //This case should never happen unless the optimizer has bugs
    assert(!(mtac::isInt(*quadruple.arg1) && mtac::isInt(*quadruple.arg2)));

    //Form  x = x * y
    if(*quadruple.arg1 == quadruple.result){
        auto reg = manager.get_pseudo_reg(quadruple.result);
        bb->emplace_back_low(ltac::Operator::MUL2, reg, to_arg(*quadruple.arg2));
    }
    //Form x = y * x
    else if(*quadruple.arg2 == quadruple.result){
        auto reg = manager.get_pseudo_reg(quadruple.result);
        bb->emplace_back_low(ltac::Operator::MUL2, reg, to_arg(*quadruple.arg1));
    }
    //Form x = y * z (z: immediate)
    else if(mtac::isVariable(*quadruple.arg1) && mtac::isInt(*quadruple.arg2)){
        bb->emplace_back_low(ltac::Operator::MUL3, manager.get_pseudo_reg_no_move(quadruple.result), to_arg(*quadruple.arg1), to_arg(*quadruple.arg2));
    }
    //Form x = y * z (y: immediate)
    else if(mtac::isInt(*quadruple.arg1) && mtac::isVariable(*quadruple.arg2)){
        bb->emplace_back_low(ltac::Operator::MUL3, manager.get_pseudo_reg_no_move(quadruple.result), to_arg(*quadruple.arg2), to_arg(*quadruple.arg1));
    }
    //Form x = y * z (both variables)
    else if(mtac::isVariable(*quadruple.arg1) && mtac::isVariable(*quadruple.arg2)){
        auto reg = manager.get_pseudo_reg_no_move(quadruple.result);
        manager.copy(*quadruple.arg1, reg);
        bb->emplace_back_low(ltac::Operator::MUL2, reg, to_arg(*quadruple.arg2));
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_DIV(mtac::Quadruple& quadruple){
    //This optimization cannot be done in the peephole optimizer
    //Form x = x / y when y is power of two
    if(*quadruple.arg1 == quadruple.result && mtac::isInt(*quadruple.arg2)){
        int constant = boost::get<int>(*quadruple.arg2);

        if(isPowerOfTwo(constant)){
            bb->emplace_back_low(ltac::Operator::SHIFT_RIGHT, manager.get_pseudo_reg(quadruple.result), powerOfTwo(constant));

            manager.set_written(quadruple.result);

            return;
        }
    }

    auto result_reg = manager.get_pseudo_reg_no_move(quadruple.result);
    auto a_reg = manager.get_bound_pseudo_reg(descriptor->a_register());
    auto d_reg = manager.get_bound_pseudo_reg(descriptor->d_register());

    manager.copy(*quadruple.arg1, a_reg);

    bb->emplace_back_low(ltac::Operator::MOV, d_reg, a_reg);
    bb->emplace_back_low(ltac::Operator::SHIFT_RIGHT, d_reg, static_cast<int>(INT->size(platform) * 8 - 1));

    if(mtac::isInt(*quadruple.arg2)){
        auto reg = manager.get_free_pseudo_reg();
        manager.move(*quadruple.arg2, reg);

        ltac::Instruction instruction(ltac::Operator::DIV, reg);
        instruction.uses.push_back(a_reg);
        instruction.uses.push_back(d_reg);
        bb->push_back(std::move(instruction));
    } else {
        ltac::Instruction instruction(ltac::Operator::DIV, to_arg(*quadruple.arg2));
        instruction.uses.push_back(a_reg);
        instruction.uses.push_back(d_reg);
        bb->push_back(std::move(instruction));
    }
    
    bb->emplace_back_low(ltac::Operator::MOV, result_reg, a_reg);

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_MOD(mtac::Quadruple& quadruple){
    auto result_reg = manager.get_pseudo_reg_no_move(quadruple.result);
    auto a_reg = manager.get_bound_pseudo_reg(descriptor->a_register());
    auto d_reg = manager.get_bound_pseudo_reg(descriptor->d_register());

    manager.copy(*quadruple.arg1, a_reg);

    bb->emplace_back_low(ltac::Operator::MOV, d_reg, a_reg);
    bb->emplace_back_low(ltac::Operator::SHIFT_RIGHT, d_reg, static_cast<int>(INT->size(platform) * 8 - 1));

    if(mtac::isInt(*quadruple.arg2)){
        auto reg = manager.get_free_pseudo_reg();
        manager.move(*quadruple.arg2, reg);

        ltac::Instruction instruction(ltac::Operator::DIV, reg);
        instruction.uses.push_back(a_reg);
        instruction.uses.push_back(d_reg);
        bb->push_back(std::move(instruction));
    } else {
        ltac::Instruction instruction(ltac::Operator::DIV, to_arg(*quadruple.arg2));
        instruction.uses.push_back(a_reg);
        instruction.uses.push_back(d_reg);
        bb->push_back(std::move(instruction));
    }

    bb->emplace_back_low(ltac::Operator::MOV, result_reg, d_reg);

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_FADD(mtac::Quadruple& quadruple){
    auto result = quadruple.result;

    //Optimize the special form a = a + b
    if(*quadruple.arg1 == result){
        auto reg = manager.get_pseudo_float_reg(result);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FADD, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FADD, reg, to_arg(*quadruple.arg2));
        }
    }
    //Optimize the special form a = b + a by using only one instruction
    else if(*quadruple.arg2 == result){
        auto reg = manager.get_pseudo_float_reg(result);

        if(mtac::isFloat(*quadruple.arg1)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg1, reg2);
            bb->emplace_back_low(ltac::Operator::FADD, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FADD, reg, to_arg(*quadruple.arg1));
        }
    }
    //In the other forms, use two instructions
    else {
        auto reg = manager.get_pseudo_float_reg_no_move(result);
        manager.copy(*quadruple.arg1, reg);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FADD, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FADD, reg, to_arg(*quadruple.arg2));
        }
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_FSUB(mtac::Quadruple& quadruple){
    auto result = quadruple.result;

    //Optimize the special form a = a - b
    if(*quadruple.arg1 == result){
        auto reg = manager.get_pseudo_float_reg(result);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FSUB, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FSUB, reg, to_arg(*quadruple.arg2));
        }
    } else {
        auto reg = manager.get_pseudo_float_reg_no_move(result);
        manager.copy(*quadruple.arg1, reg);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FSUB, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FSUB, reg, to_arg(*quadruple.arg2));
        }
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_FMUL(mtac::Quadruple& quadruple){
    //Form  x = x * y
    if(*quadruple.arg1 == quadruple.result){
        auto reg = manager.get_pseudo_float_reg(quadruple.result);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FMUL, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FMUL, reg, to_arg(*quadruple.arg2));
        }
    }
    //Form x = y * x
    else if(*quadruple.arg2 == quadruple.result){
        auto reg = manager.get_pseudo_float_reg(quadruple.result);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FMUL, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FMUL, reg, to_arg(*quadruple.arg2));
        }
    } 
    //General form
    else  {
        auto reg = manager.get_pseudo_float_reg_no_move(quadruple.result);
        manager.copy(*quadruple.arg1, reg);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FMUL, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FMUL, reg, to_arg(*quadruple.arg2));
        }
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_FDIV(mtac::Quadruple& quadruple){
    //Form x = x / y
    if(*quadruple.arg1 == quadruple.result){
        auto reg = manager.get_pseudo_float_reg(quadruple.result);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FDIV, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FDIV, reg, to_arg(*quadruple.arg2));
        }
    } 
    //General form
    else {
        auto reg = manager.get_pseudo_float_reg_no_move(quadruple.result);
        manager.copy(*quadruple.arg1, reg);

        if(mtac::isFloat(*quadruple.arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple.arg2, reg2);
            bb->emplace_back_low(ltac::Operator::FDIV, reg, reg2);
        } else {
            bb->emplace_back_low(ltac::Operator::FDIV, reg, to_arg(*quadruple.arg2));
        }
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_EQUALS(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVE, quadruple, false);
}

void ltac::StatementCompiler::compile_NOT_EQUALS(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVNE, quadruple, false);
}

void ltac::StatementCompiler::compile_GREATER(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVG, quadruple, false);
}

void ltac::StatementCompiler::compile_GREATER_EQUALS(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVGE, quadruple, false);
}

void ltac::StatementCompiler::compile_LESS(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVL, quadruple, false);
}

void ltac::StatementCompiler::compile_LESS_EQUALS(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVLE, quadruple, false);
}

void ltac::StatementCompiler::compile_FE(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVE, quadruple, true);
}

void ltac::StatementCompiler::compile_FNE(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVNE, quadruple, true);
}

void ltac::StatementCompiler::compile_FG(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVA, quadruple, true);
}

void ltac::StatementCompiler::compile_FGE(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVAE, quadruple, true);
}

void ltac::StatementCompiler::compile_FLE(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVBE, quadruple, true);
}

void ltac::StatementCompiler::compile_FL(mtac::Quadruple& quadruple){
    set_if_cc(ltac::Operator::CMOVB, quadruple, true);
}

void ltac::StatementCompiler::compile_MINUS(mtac::Quadruple& quadruple){
    //Constants should have been replaced by the optimizer
    assert(mtac::isVariable(*quadruple.arg1));

    auto var = ltac::get_variable(*quadruple.arg1);

    if(quadruple.result == var){
        bb->emplace_back_low(ltac::Operator::NEG, manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg1)));
    } else {
        auto reg = manager.get_pseudo_reg_no_move(quadruple.result);
        bb->emplace_back_low(ltac::Operator::MOV, reg, manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg1)));
        bb->emplace_back_low(ltac::Operator::NEG, reg);
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_FMINUS(mtac::Quadruple& quadruple){
    //Constants should have been replaced by the optimizer
    assert(mtac::isVariable(*quadruple.arg1));
    auto var = ltac::get_variable(*quadruple.arg1);
    auto var_reg = manager.get_pseudo_float_reg(var);

    auto reg = manager.get_free_pseudo_float_reg();
    manager.copy(-1.0, reg);

    if(quadruple.result == var){
        bb->emplace_back_low(ltac::Operator::FMUL, var_reg, reg);
    } else {
        auto result_reg = manager.get_pseudo_float_reg_no_move(quadruple.result);
        bb->emplace_back_low(ltac::Operator::FMOV, result_reg, var_reg);
        bb->emplace_back_low(ltac::Operator::FMUL, result_reg, reg);
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_I2F(mtac::Quadruple& quadruple){
    //Constants should have been replaced by the optimizer
    assert(mtac::isVariable(*quadruple.arg1));

    auto reg = manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg1));
    auto result_reg = manager.get_pseudo_float_reg_no_move(quadruple.result);

    bb->emplace_back_low(ltac::Operator::I2F, result_reg, reg);

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_F2I(mtac::Quadruple& quadruple){
    //Constants should have been replaced by the optimizer
    assert(mtac::isVariable(*quadruple.arg1));

    auto reg = manager.get_pseudo_float_reg(ltac::get_variable(*quadruple.arg1));
    auto result_reg = manager.get_pseudo_reg_no_move(quadruple.result);

    bb->emplace_back_low(ltac::Operator::F2I, result_reg, reg);

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_DOT(mtac::Quadruple& quadruple){
    auto size = convert_size(quadruple.size);
    
    if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
        auto variable = *var_ptr;

        if(variable->type()->is_pointer() || (variable->type()->is_dynamic_array() && !variable->position().isParameter())){
            auto reg = manager.get_pseudo_reg_no_move(quadruple.result);
            
            //TODO if the reg is already 8-bit allocatable, use it directly
            
            //Necessary to obtain an hard reg here to be sure that it 8-bit allocatable
            auto hard_reg = manager.get_bound_pseudo_reg(descriptor->d_register());
            
            ltac::Instruction instruction(ltac::Operator::MOV, hard_reg, address(variable, *quadruple.arg2));
            instruction.size = size;
            bb->push_back(std::move(instruction));
            
            bb->emplace_back_low(ltac::Operator::MOV, reg, hard_reg);

            uses.push_back(hard_reg);
        } else {
            if(ltac::is_float_var(quadruple.result)){
                auto reg = manager.get_pseudo_float_reg_no_move(quadruple.result);
                ltac::Instruction instruction(ltac::Operator::FMOV, reg, address(variable, *quadruple.arg2));
                instruction.size = size;
                bb->push_back(std::move(instruction));
            } else {
                auto reg = manager.get_pseudo_reg_no_move(quadruple.result);

                //TODO if the reg is already 8-bit allocatable, use it directly

                //Necessary to obtain an hard reg here to be sure that it 8-bit allocatable
                auto hard_reg = manager.get_bound_pseudo_reg(descriptor->d_register());
                
                ltac::Instruction instruction(ltac::Operator::MOV, hard_reg, address(variable, *quadruple.arg2));
                instruction.size = size;
                bb->push_back(std::move(instruction));

                bb->emplace_back_low(ltac::Operator::MOV, reg, hard_reg);

                uses.push_back(hard_reg);
            }
        }
    } else if(auto* string_ptr = boost::get<std::string>(&*quadruple.arg1)){
        auto reg = manager.get_pseudo_reg_no_move(quadruple.result);

        if(auto* offset_ptr = boost::get<int>(&*quadruple.arg2)){
            ltac::Instruction instruction(ltac::Operator::MOV, reg, ltac::Address(*string_ptr, *offset_ptr));
            instruction.size = size;
            bb->push_back(std::move(instruction));
        } else if(auto* offset_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
            auto offset_reg = manager.get_pseudo_reg(*offset_ptr);
            ltac::Instruction instruction(ltac::Operator::MOV, reg, ltac::Address(*string_ptr, offset_reg));
            instruction.size = size;
            bb->push_back(std::move(instruction));
        }
    }

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_FDOT(mtac::Quadruple& quadruple){
    assert(boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1));
    auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple.arg1);

    assert(boost::get<int>(&*quadruple.arg2));
    int offset = boost::get<int>(*quadruple.arg2);

    auto reg = manager.get_pseudo_float_reg_no_move(quadruple.result);
    bb->emplace_back_low(ltac::Operator::FMOV, reg, address(variable, offset));

    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_PDOT(mtac::Quadruple& quadruple){
    assert(boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1));
    auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple.arg1);

    auto reg = manager.get_pseudo_reg_no_move(quadruple.result);

    //The pointer has to be dereferenced
    if(variable->type()->is_pointer()){
        auto ptr_reg = manager.get_pseudo_reg(variable);

        if(mtac::is<int>(*quadruple.arg2)){
            int offset = boost::get<int>(*quadruple.arg2);
            bb->emplace_back_low(ltac::Operator::LEA, reg, ltac::Address(ptr_reg, offset));
        } else {
            assert(ltac::is_variable(*quadruple.arg2));

            auto offset = manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg2));
            bb->emplace_back_low(ltac::Operator::LEA, reg, ltac::Address(ptr_reg, offset));
        }
    } else {
        if(mtac::is<int>(*quadruple.arg2)){
            int offset = boost::get<int>(*quadruple.arg2);

            auto reg2 = get_address_in_pseudo_reg(variable, offset);
            bb->emplace_back_low(ltac::Operator::MOV, reg, reg2);
        } else {
            assert(ltac::is_variable(*quadruple.arg2));

            auto offset = manager.get_pseudo_reg(ltac::get_variable(*quadruple.arg2));
            auto reg2 = get_address_in_pseudo_reg2(variable, offset);

            bb->emplace_back_low(ltac::Operator::MOV, reg, reg2);
        }
    }
    
    manager.set_written(quadruple.result);
}

void ltac::StatementCompiler::compile_DOT_ASSIGN(mtac::Quadruple& quadruple){
    if(quadruple.size == mtac::Size::BYTE){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
            auto reg = manager.get_pseudo_reg(*ptr);

            //TODO if the reg is already 8-bit allocatable, use it directly
                
            //Necessary to obtain an hard reg here to be sure that it 8-bit allocatable
            auto hard_reg = manager.get_bound_pseudo_reg(descriptor->d_register());
            bb->emplace_back_low(ltac::Operator::MOV, hard_reg, reg);

            ltac::Instruction mov(ltac::Operator::MOV, address(quadruple.result, *quadruple.arg1), hard_reg);
            mov.size = ltac::Size::BYTE;
            bb->push_back(std::move(mov));

            uses.push_back(hard_reg);
        } else {
            ltac::Instruction mov(ltac::Operator::MOV, address(quadruple.result, *quadruple.arg1), to_arg(*quadruple.arg2));
            mov.size = convert_size(quadruple.size);
            bb->push_back(std::move(mov));
        }
    } else {
        ltac::Instruction mov(ltac::Operator::MOV, address(quadruple.result, *quadruple.arg1), to_arg(*quadruple.arg2));
        mov.size = convert_size(quadruple.size);
        bb->push_back(std::move(mov));
    }
}

void ltac::StatementCompiler::compile_DOT_FASSIGN(mtac::Quadruple& quadruple){
    auto reg = manager.get_free_pseudo_float_reg();
    manager.copy(*quadruple.arg2, reg);
    bb->emplace_back_low(ltac::Operator::FMOV, address(quadruple.result, *quadruple.arg1), reg);
}

void ltac::StatementCompiler::compile_DOT_PASSIGN(mtac::Quadruple& quadruple){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg2)){
        auto variable = *ptr;

        auto reg = get_address_in_pseudo_reg(variable, 0);
        bb->emplace_back_low(ltac::Operator::MOV, address(quadruple.result, *quadruple.arg1), reg); 
    } else if(mtac::is<int>(*quadruple.arg2)){
        bb->emplace_back_low(ltac::Operator::MOV, address(quadruple.result, *quadruple.arg1), boost::get<int>(*quadruple.arg2)); 
    } else {
        eddic_unreachable("Unsupported rhs type in DOT_PASSIGN");
    }
}

void ltac::StatementCompiler::compile_NOT(mtac::Quadruple& quadruple){
    auto reg = manager.get_pseudo_reg_no_move(quadruple.result);
    manager.copy(*quadruple.arg1, reg);
    bb->emplace_back_low(ltac::Operator::NOT, reg); 
}

void ltac::StatementCompiler::compile_AND(mtac::Quadruple& quadruple){
    auto reg = manager.get_pseudo_reg_no_move(quadruple.result);
    manager.copy(*quadruple.arg1, reg);
    bb->emplace_back_low(ltac::Operator::AND, reg, boost::get<int>(*quadruple.arg2));
}

void ltac::StatementCompiler::compile_RETURN(mtac::Quadruple& quadruple){
    std::vector<ltac::PseudoRegister> uses;
    std::vector<ltac::PseudoFloatRegister> float_uses;

    //A return without args is the same as exiting from the function
    if(quadruple.arg1){
        if(mtac::isFloat(*quadruple.arg1)){
            auto return_reg = manager.get_bound_pseudo_float_reg(descriptor->float_return_register());
            manager.move(*quadruple.arg1, return_reg);
            float_uses.push_back(return_reg);
        } else if(boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1) && ltac::is_float_var(ltac::get_variable(*quadruple.arg1))){
            auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple.arg1);

            auto reg = manager.get_pseudo_float_reg(variable);
            auto return_reg = manager.get_bound_pseudo_float_reg(descriptor->float_return_register());
            
            bb->emplace_back_low(ltac::Operator::FMOV, return_reg, reg);
            float_uses.push_back(return_reg);
        } else {
            auto reg1 = manager.get_bound_pseudo_reg(descriptor->int_return_register1());
            bb->emplace_back_low(ltac::Operator::MOV, reg1, to_arg(*quadruple.arg1));
            uses.push_back(reg1);

            if(quadruple.arg2){
                auto reg2 = manager.get_bound_pseudo_reg(descriptor->int_return_register2());
                bb->emplace_back_low(ltac::Operator::MOV, reg2, to_arg(*quadruple.arg2));
                uses.push_back(reg2);
            }
        }
    }

    end_bb();

    ltac::Instruction instruction(ltac::Operator::PRE_RET);
    instruction.uses = uses;
    instruction.float_uses = float_uses;
    bb->push_back(std::move(instruction));
}

void ltac::StatementCompiler::compile(mtac::Quadruple& quadruple){
    LOG<Trace>("Compiler") << "Current statement " << quadruple << log::endl;

    switch(quadruple.op){
        case mtac::Operator::ASSIGN:
            compile_ASSIGN(quadruple);
            break;
        case mtac::Operator::PASSIGN:
            compile_PASSIGN(quadruple);
            break;
        case mtac::Operator::FASSIGN:
            compile_FASSIGN(quadruple);
            break;
        case mtac::Operator::ADD:
            compile_ADD(quadruple);
            break;            
        case mtac::Operator::SUB:
            compile_SUB(quadruple);
            break;            
        case mtac::Operator::MUL:
            compile_MUL(quadruple);
            break;            
        case mtac::Operator::DIV:
            compile_DIV(quadruple);
            break;            
        case mtac::Operator::MOD:
            compile_MOD(quadruple);
            break;
        case mtac::Operator::FADD:
            compile_FADD(quadruple);
            break;
        case mtac::Operator::FSUB:
            compile_FSUB(quadruple);
            break;
        case mtac::Operator::FMUL:
            compile_FMUL(quadruple);
            break;
        case mtac::Operator::FDIV:
            compile_FDIV(quadruple);
            break;
        case mtac::Operator::I2F:
            compile_I2F(quadruple);
            break;
        case mtac::Operator::F2I:
            compile_F2I(quadruple);
            break;
        case mtac::Operator::MINUS:
            compile_MINUS(quadruple);
            break;
        case mtac::Operator::FMINUS:
            compile_FMINUS(quadruple);
            break;
        case mtac::Operator::GREATER:
            compile_GREATER(quadruple);
            break;
        case mtac::Operator::GREATER_EQUALS:
            compile_GREATER_EQUALS(quadruple);
            break;
        case mtac::Operator::LESS:
            compile_LESS(quadruple);
            break;
        case mtac::Operator::LESS_EQUALS:
            compile_LESS_EQUALS(quadruple);
            break;
        case mtac::Operator::EQUALS:
            compile_EQUALS(quadruple);
            break;
        case mtac::Operator::NOT_EQUALS:
            compile_NOT_EQUALS(quadruple);
            break;
        case mtac::Operator::FG:
            compile_FG(quadruple);
            break;
        case mtac::Operator::FGE:
            compile_FGE(quadruple);
            break;
        case mtac::Operator::FL:
            compile_FL(quadruple);
            break;
        case mtac::Operator::FLE:
            compile_FLE(quadruple);
            break;
        case mtac::Operator::FE:
            compile_FE(quadruple);
            break;
        case mtac::Operator::FNE:
            compile_FNE(quadruple);
            break;
        case mtac::Operator::DOT:
            compile_DOT(quadruple);
            break;
        case mtac::Operator::PDOT:
            compile_PDOT(quadruple);
            break;
        case mtac::Operator::FDOT:
            compile_FDOT(quadruple);
            break;
        case mtac::Operator::DOT_ASSIGN:
            compile_DOT_ASSIGN(quadruple);
            break;
        case mtac::Operator::DOT_PASSIGN:
            compile_DOT_PASSIGN(quadruple);
            break;
        case mtac::Operator::DOT_FASSIGN:
            compile_DOT_FASSIGN(quadruple);
            break;
        case mtac::Operator::RETURN:
            compile_RETURN(quadruple);
            break;
        case mtac::Operator::NOT:
            compile_NOT(quadruple);
            break;
        case mtac::Operator::AND:
            compile_AND(quadruple);
            break;
        case mtac::Operator::GOTO:
            compile_GOTO(quadruple);
            break;
        case mtac::Operator::PARAM:
            compile_PARAM(quadruple);
            break;
        case mtac::Operator::PPARAM:
            compile_PPARAM(quadruple);
            break;
        case mtac::Operator::CALL:
            compile_CALL(quadruple);
            break;
        case mtac::Operator::LABEL:
            bb->emplace_back_low(quadruple.label(), ltac::Operator::LABEL);
            break;
        case mtac::Operator::IF_FALSE_UNARY:
            compare_unary(*quadruple.arg1);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::Z);
            break;
        case mtac::Operator::IF_FALSE_FE:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::NE);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FALSE_FNE:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::E);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FALSE_FL:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::AE);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FALSE_FLE:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::A);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FALSE_FG:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::BE);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FALSE_FGE:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::B);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FALSE_EQUALS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::NE);
            break;
        case mtac::Operator::IF_FALSE_NOT_EQUALS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::E);
            break;
        case mtac::Operator::IF_FALSE_LESS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::GE);
            break;
        case mtac::Operator::IF_FALSE_LESS_EQUALS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::G);
            break;
        case mtac::Operator::IF_FALSE_GREATER:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::LE);
            break;
        case mtac::Operator::IF_FALSE_GREATER_EQUALS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::L);
            break;
        case mtac::Operator::IF_UNARY:
            compare_unary(*quadruple.arg1);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::NZ);
            break;
        case mtac::Operator::IF_FE:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::E);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FNE:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::NE);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FL:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::B);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FLE:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::BE);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FG:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::A);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_FGE:
            compare_float_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::AE);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::P);
            break;
        case mtac::Operator::IF_EQUALS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::E);
            break;
        case mtac::Operator::IF_NOT_EQUALS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::NE);
            break;
        case mtac::Operator::IF_LESS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::L);
            break;
        case mtac::Operator::IF_LESS_EQUALS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::LE);
            break;
        case mtac::Operator::IF_GREATER:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::G);
            break;
        case mtac::Operator::IF_GREATER_EQUALS:
            compare_binary(*quadruple.arg1, *quadruple.arg2);
            bb->emplace_back_low(quadruple.block->label, ltac::Operator::GE);
            break;
        case mtac::Operator::NOP:
            //No code necessary
            break;
    }
}
