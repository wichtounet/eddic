//=======================================================================
// Copyright Baptiste Wicht 2011.
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

#include "ltac/StatementCompiler.hpp"
#include "ltac/Utils.hpp"

#include "mtac/Utils.hpp" 

using namespace eddic;

namespace {

template<typename Reg>
struct register_guard {
    Reg reg;
    ltac::RegisterManager& manager;

    register_guard(Reg reg, ltac::RegisterManager& manager) : reg(reg), manager(manager) {}

    ~register_guard(){
        manager.release(reg);
    }

    operator Reg(){
        return reg;
    }
};

} //end of anonymous namespace

ltac::StatementCompiler::StatementCompiler(const std::vector<ltac::Register>& registers, const std::vector<ltac::FloatRegister>& float_registers, 
        std::shared_ptr<ltac::Function> function, std::shared_ptr<FloatPool> float_pool) : 
        manager(registers, float_registers, function, float_pool), function(function), float_pool(float_pool) {}

void ltac::StatementCompiler::set_current(mtac::Statement statement){
    manager.set_current(statement);
}

void ltac::StatementCompiler::reset(){
    manager.reset();

    ended = false;
}

void ltac::StatementCompiler::end_basic_block(){
    manager.spills_all();

    ended = true;
}

void ltac::StatementCompiler::collect_parameters(std::shared_ptr<eddic::Function> definition){
    manager.collect_parameters(definition, descriptor);
}

void ltac::StatementCompiler::collect_variables(std::shared_ptr<eddic::Function> definition){
    manager.collect_variables(definition, descriptor);
}

ltac::Register ltac::StatementCompiler::to_register(std::shared_ptr<Variable> var){
    return ltac::to_register(var, manager);
}

ltac::Argument ltac::StatementCompiler::to_arg(mtac::Argument argument){
    return ltac::to_arg(argument, manager);
}

ltac::Address ltac::StatementCompiler::stack_address(int offset){
    if(option_defined("fomit-frame-pointer")){
        return ltac::Address(ltac::SP, offset + bp_offset);
    } else {
        return ltac::Address(ltac::BP, offset);
    }
}

ltac::Address ltac::StatementCompiler::stack_address(ltac::Register offsetReg, int offset){
    if(option_defined("fomit-frame-pointer")){
        return ltac::Address(ltac::SP, offsetReg, 1, offset + bp_offset);
    } else {
        return ltac::Address(ltac::BP, offsetReg, 1, offset);
    }
}

ltac::Address ltac::StatementCompiler::to_pointer(std::shared_ptr<Variable> var, int offset){
    assert(var->type()->is_pointer());

    auto reg = manager.get_reg(var);
    return ltac::Address(reg, offset);
}

ltac::Address ltac::StatementCompiler::to_address(std::shared_ptr<Variable> var, int offset){
    auto position = var->position();

    if(position.isStack()){
        return stack_address(position.offset() + offset);
    } else if(position.isParameter()){
        //The case of array is special because only the address is passed, not the complete array
        if(var->type()->is_array())
        {
            auto reg = register_guard<ltac::Register>(manager.get_free_reg(), manager);

            ltac::add_instruction(function, ltac::Operator::MOV, reg, stack_address(position.offset()));

            return ltac::Address(reg, offset);
        }
        //In the other cases, the value is passed, so we can compute the offset directly
        else {
            return stack_address(position.offset() + offset);
        }
    } else if(position.isGlobal()){
        return ltac::Address("V" + position.name(), offset);
    } 
    
    //In the other cases, the variable is already in a register
    
    auto reg = manager.get_reg(var);
    return ltac::Address(reg, offset);
}

ltac::Address ltac::StatementCompiler::to_address(std::shared_ptr<Variable> var, mtac::Argument offset){
    if(auto* ptr = boost::get<int>(&offset)){
        return to_address(var, *ptr);
    }

    auto position = var->position();
    
    assert(ltac::is_variable(offset));
    auto offsetReg = manager.get_reg(ltac::get_variable(offset));

    if(position.isStack()){
        return stack_address(offsetReg, position.offset());
    } else if(position.isParameter()){
        auto reg = register_guard<ltac::Register>(manager.get_free_reg(), manager);

        ltac::add_instruction(function, ltac::Operator::MOV, reg, stack_address(position.offset()));

        return ltac::Address(reg, offsetReg);
    } else if(position.isGlobal()){
        return ltac::Address("V" + position.name(), offsetReg);
    } 
    
    assert(position.isTemporary());
    
    auto reg = manager.get_reg(var);
    return ltac::Address(reg, offsetReg);
}

void ltac::StatementCompiler::pass_in_int_register(mtac::Argument& argument, int position){
    ltac::add_instruction(function, ltac::Operator::MOV, ltac::Register(descriptor->int_param_register(position)), to_arg(argument));
}

void ltac::StatementCompiler::pass_in_float_register(mtac::Argument& argument, int position){
    if(auto* ptr = boost::get<int>(&argument)){
        auto label = float_pool->label(static_cast<double>(*ptr));
        ltac::add_instruction(function, ltac::Operator::FMOV, ltac::FloatRegister(descriptor->float_param_register(position)), ltac::Address(label));
    } else if(auto* ptr = boost::get<double>(&argument)){
        auto label = float_pool->label(*ptr);
        ltac::add_instruction(function, ltac::Operator::FMOV, ltac::FloatRegister(descriptor->float_param_register(position)), ltac::Address(label));
    } else {
        ltac::add_instruction(function, ltac::Operator::FMOV, ltac::FloatRegister(descriptor->float_param_register(position)), to_arg(argument));
    }
}

void ltac::StatementCompiler::compare_binary(mtac::Argument& arg1, mtac::Argument& arg2){
    //The first argument is not important, it can be immediate, but the second must be a register
    if(auto* ptr = boost::get<int>(&arg1)){
        auto reg1 = register_guard<ltac::Register>(manager.get_free_reg(), manager);

        ltac::add_instruction(function, ltac::Operator::MOV, reg1, *ptr);

        auto reg2 = manager.get_reg(ltac::get_variable(arg2));

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::CMP_INT, reg1, reg2);
    } else {
        auto reg1 = manager.get_reg(ltac::get_variable(arg1));
        auto reg2 = to_arg(arg2);

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::CMP_INT, reg1, reg2);
    }
}

void ltac::StatementCompiler::compare_float_binary(mtac::Argument& arg1, mtac::Argument& arg2){
    //Comparisons of constant should have been handled by the optimizer
    assert(!(isFloat(arg1) && isFloat(arg2))); 

    //If both args are variables
    if(isVariable(arg1) && isVariable(arg2)){
        //The basic block must be ended before the jump
        end_basic_block();

        auto reg1 = manager.get_float_reg(ltac::get_variable(arg1));
        auto reg2 = manager.get_float_reg(ltac::get_variable(arg2));

        ltac::add_instruction(function, ltac::Operator::CMP_FLOAT, reg1, reg2);
    } else if(isVariable(arg1) && isFloat(arg2)){
        auto reg1 = manager.get_float_reg(ltac::get_variable(arg1));
        auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);

        manager.copy(arg2, reg2);

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::CMP_FLOAT, reg1, reg2);
    } else if(isFloat(arg1) && isVariable(arg2)){
        auto reg1 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
        auto reg2 = manager.get_float_reg(ltac::get_variable(arg2));

        manager.copy(arg1, reg1);

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::CMP_FLOAT, reg1, reg2);
    }
}

void ltac::StatementCompiler::compare_unary(mtac::Argument arg1){
    if(auto* ptr = boost::get<int>(&arg1)){
        auto reg = register_guard<ltac::Register>(manager.get_free_reg(), manager);

        ltac::add_instruction(function, ltac::Operator::MOV, reg, *ptr);

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::OR, reg, reg);
    } else {
        //The basic block must be ended before the jump
        end_basic_block();

        auto reg = manager.get_reg(ltac::get_variable(arg1));

        ltac::add_instruction(function, ltac::Operator::OR, reg, reg);
    }
}

//Div eax by arg2 
void ltac::StatementCompiler::div_eax(std::shared_ptr<mtac::Quadruple> quadruple){
    ltac::add_instruction(function, ltac::Operator::MOV, ltac::Register(descriptor->d_register()), ltac::Register(descriptor->a_register()));
    ltac::add_instruction(function, ltac::Operator::SHIFT_RIGHT, ltac::Register(descriptor->d_register()), static_cast<int>(INT->size() * 8 - 1));

    if(isInt(*quadruple->arg2)){
        auto reg = manager.get_free_reg();
        manager.move(*quadruple->arg2, reg);

        ltac::add_instruction(function, ltac::Operator::DIV, reg);

        if(manager.is_reserved(reg)){
            manager.release(reg);
        }
    } else {
        ltac::add_instruction(function, ltac::Operator::DIV, to_arg(*quadruple->arg2));
    }
}

void ltac::StatementCompiler::set_if_cc(ltac::Operator set, std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_reg_no_move(quadruple->result);

    //The first argument is not important, it can be immediate, but the second must be a register
    if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
        auto reg = register_guard<ltac::Register>(manager.get_free_reg(), manager);

        ltac::add_instruction(function, ltac::Operator::MOV, reg, *ptr); 
        ltac::add_instruction(function, ltac::Operator::CMP_INT, reg, to_arg(*quadruple->arg2)); 
    } else {
        ltac::add_instruction(function, ltac::Operator::CMP_INT, to_arg(*quadruple->arg1), to_arg(*quadruple->arg2)); 
    }

    //Conditionally move 1 in the register
    auto value_reg = register_guard<ltac::Register>(manager.get_free_reg(), manager);
    ltac::add_instruction(function, ltac::Operator::MOV, value_reg, 1); 
    ltac::add_instruction(function, set, reg, value_reg); 

    manager.set_written(quadruple->result);
}
        
void ltac::StatementCompiler::push(ltac::Argument arg){
    ltac::add_instruction(function, ltac::Operator::PUSH, arg);
    bp_offset += INT->size();
}

void ltac::StatementCompiler::pop(ltac::Argument arg){
    ltac::add_instruction(function, ltac::Operator::POP, arg);
    bp_offset -= INT->size();
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::IfFalse> if_false){
    manager.set_current(if_false);

    if(if_false->op){
        //Depending on the type of the operator, do a float or a int comparison
        if(ltac::is_float_operator(*if_false->op)){
            compare_float_binary(if_false->arg1, *if_false->arg2);

            switch(*if_false->op){
                case mtac::BinaryOperator::FE:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::NE));
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FNE:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::E));
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FL:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::AE));
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FLE:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::A));
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FG:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::BE));
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FGE:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::B));
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                default:
                    ASSERT_PATH_NOT_TAKEN("This operation is not a float operator");
                    break;
            }
        } else {
            compare_binary(if_false->arg1, *if_false->arg2);

            switch(*if_false->op){
                case mtac::BinaryOperator::EQUALS:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::NE));
                    break;
                case mtac::BinaryOperator::NOT_EQUALS:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::E));
                    break;
                case mtac::BinaryOperator::LESS:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::GE));
                    break;
                case mtac::BinaryOperator::LESS_EQUALS:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::G));
                    break;
                case mtac::BinaryOperator::GREATER:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::LE));
                    break;
                case mtac::BinaryOperator::GREATER_EQUALS:
                    function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::L));
                    break;
                default:
                    ASSERT_PATH_NOT_TAKEN("This operation is not a float operator");
                    break;
            }
        }
    } else {
        compare_unary(if_false->arg1);

        function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::Z));
    }

    offset_labels[if_false->block->label] = bp_offset;
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::If> if_){
    manager.set_current(if_);

    if(if_->op){
        //Depending on the type of the operator, do a float or a int comparison
        if(ltac::is_float_operator(*if_->op)){
            compare_float_binary(if_->arg1, *if_->arg2);

            switch(*if_->op){
                case mtac::BinaryOperator::FE:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::E));
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FNE:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::NE));
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FL:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::B));
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FLE:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::BE));
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FG:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::A));
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FGE:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::AE));
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                default:
                    ASSERT_PATH_NOT_TAKEN("This operation is not a float operator");
                    break;
            }
        } else {
            compare_binary(if_->arg1, *if_->arg2);

            switch(*if_->op){
                case mtac::BinaryOperator::EQUALS:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::E));
                    break;
                case mtac::BinaryOperator::NOT_EQUALS:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::NE));
                    break;
                case mtac::BinaryOperator::LESS:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::L));
                    break;
                case mtac::BinaryOperator::LESS_EQUALS:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::LE));
                    break;
                case mtac::BinaryOperator::GREATER:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::G));
                    break;
                case mtac::BinaryOperator::GREATER_EQUALS:
                    function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::GE));
                    break;
                default:
                    ASSERT_PATH_NOT_TAKEN("This operation is not a float operator");
                    break;
            }
        }

    } else {
        compare_unary(if_->arg1);

        function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::NZ));
    }
    
    offset_labels[if_->block->label] = bp_offset;
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Goto> goto_){
    manager.set_current(goto_);

    //The basic block must be ended before the jump
    end_basic_block();

    function->add(std::make_shared<ltac::Jump>(goto_->block->label, ltac::JumpType::ALWAYS));
}

inline ltac::Register ltac::StatementCompiler::get_address_in_reg2(std::shared_ptr<Variable> var, ltac::Register offset){
    auto reg = manager.get_free_reg();
    
    ltac::add_instruction(function, ltac::Operator::LEA, reg, to_address(var, 0));
    ltac::add_instruction(function, ltac::Operator::ADD, reg, offset);
    
    return reg;
}

inline ltac::Register ltac::StatementCompiler::get_address_in_reg(std::shared_ptr<Variable> var, int offset){
    auto reg = manager.get_free_reg();

    ltac::add_instruction(function, ltac::Operator::LEA, reg, to_address(var, offset));
    
    return reg;
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Param> param){
    manager.set_current(param);
    manager.save_registers(param, descriptor);
    
    std::shared_ptr<const Type> type;
    bool register_allocated = false;
    unsigned int position = 0;
        
    if(param->std_param.length() > 0 || (param->param && option_defined("fparameter-allocation"))){
        unsigned int maxInt = descriptor->numberOfIntParamRegisters();
        unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();

        //It's a call to a standard function
        if(param->std_param.length() > 0){
            type = param->function->getParameterType(param->std_param);
            position = param->function->getParameterPositionByType(param->std_param);
        } 
        //It's a call to a user function
        else if(param->param){
            type = param->param->type();
            position = param->function->getParameterPositionByType(param->param->name());
        }

        register_allocated = 
                (mtac::is_single_int_register(type) && position <= maxInt)
            ||  (mtac::is_single_float_register(type) && position <= maxFloat);
    }

    //Push the address of the var
    if(param->address){
        auto variable = boost::get<std::shared_ptr<Variable>>(param->arg);

        if(variable->type()->is_pointer()){
            auto reg = manager.get_reg(variable);

            if(register_allocated){
                ltac::add_instruction(function, ltac::Operator::MOV, ltac::Register(descriptor->int_param_register(position)), reg);
            } else {
                push(reg);
            }
        } else {
            auto reg = register_guard<ltac::Register>(get_address_in_reg(variable, 0), manager);

            if(register_allocated){
                ltac::add_instruction(function, ltac::Operator::MOV, ltac::Register(descriptor->int_param_register(position)), reg);
            } else {
                push(reg);
            }
        }
    } 
    //Push by value
    else {
        if(register_allocated){
            if(auto* ptr = boost::get<int>(&param->arg)){
                if(*ptr == 0){
                    if(param->param && param->param->type() == FLOAT){
                        pass_in_float_register(param->arg, position);
                        return;
                    } else if(!param->std_param.empty() && param->function->getParameterType(param->std_param) == FLOAT){
                        pass_in_float_register(param->arg, position);
                        return;
                    } 
                } 
            }

            if(mtac::is_single_int_register(type)){
                pass_in_int_register(param->arg, position);
            } else {
                pass_in_float_register(param->arg, position);
            }

            return;
        }

        //If the param as not been handled as register passing, push it on the stack 
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param->arg)){
            if(!(*ptr)->type()->is_array() && ltac::is_float_var(*ptr)){
                auto reg1 = register_guard<ltac::Register>(manager.get_free_reg(), manager);
                auto reg2 = manager.get_float_reg(*ptr);

                ltac::add_instruction(function, ltac::Operator::MOV, reg1, reg2);
                push(reg1);
            } else {
                if((*ptr)->type()->is_array()){
                    auto position = (*ptr)->position();

                    if(position.isGlobal()){
                        auto reg = register_guard<ltac::Register>(manager.get_free_reg(), manager);

                        ltac::add_instruction(function, ltac::Operator::MOV, reg, "V" + position.name());
                        push(reg);
                    } else if(position.isStack()){
                        auto reg = register_guard<ltac::Register>(manager.get_free_reg(), manager);

                        ltac::add_instruction(function, ltac::Operator::LEA, reg, stack_address(position.offset()));
                        push(reg);
                    } else if(position.isParameter()){
                        push(stack_address(position.offset()));
                    }
                } else {
                    auto reg = manager.get_reg(ltac::get_variable(param->arg));
                    push(reg);
                }
            }
        } else if(auto* ptr = boost::get<int>(&param->arg)){
            if(*ptr == 0){
                if(param->param && param->param->type() == FLOAT){
                    auto label = float_pool->label(0.0);
                    push(ltac::Address(label));
                } else if(!param->std_param.empty() && param->function->getParameterType(param->std_param) == FLOAT){
                    auto label = float_pool->label(0.0);
                    push(ltac::Address(label));
                } else {
                    push(to_arg(param->arg));
                }
            } else {
                push(to_arg(param->arg));
            }
        } else if(auto* ptr = boost::get<double>(&param->arg)){
            auto label = float_pool->label(*ptr);
            push(ltac::Address(label));
        } else {
            push(to_arg(param->arg));
        }
    }
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Call> call){
    manager.set_current(call);

    function->add(std::make_shared<ltac::Jump>(call->function, ltac::JumpType::CALL));

    int total = 0;

    unsigned int maxInt = descriptor->numberOfIntParamRegisters();
    unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();
    
    if(!call->functionDefinition->standard && !option_defined("fparameter-allocation")){
        maxInt = 0;
        maxFloat = 0;
    }

    for(auto& param : call->functionDefinition->parameters){
        auto type = param.paramType; 

        if(type->is_array()){
            //Passing an array is just passing an adress
            total += INT->size();
        } else {
            if(mtac::is_single_int_register(type)){
                //If the parameter is allocated in a register, there is no need to deallocate stack space for it
                if(maxInt > 0){
                    --maxInt;
                } else {
                    total += type->size();
                }
            } else if(mtac::is_single_float_register(type)){
                //If the parameter is allocated in a register, there is no need to deallocate stack space for it
                if(maxFloat > 0){
                    --maxFloat;
                } else {
                    total += type->size();
                }
            } else {
                total += type->size();
            }
        }
    }

    ltac::add_instruction(function, ltac::Operator::ADD, ltac::SP, total);
    bp_offset -= total;

    if(call->return_){
        if(call->return_->type() == FLOAT){
            if(call->return_->position().is_register()){
                ltac::add_instruction(function, ltac::Operator::MOV, manager.get_float_reg_no_move(call->return_), ltac::FloatRegister(descriptor->float_return_register()));
            } else {
                manager.setLocation(call->return_, ltac::FloatRegister(descriptor->float_return_register()));
            }
        } else {
            if(call->return_->position().is_register()){
                ltac::add_instruction(function, ltac::Operator::MOV, manager.get_reg_no_move(call->return_), ltac::Register(descriptor->int_return_register1()));
            } else {
                manager.setLocation(call->return_, ltac::Register(descriptor->int_return_register1()));
            }
        }

        manager.set_written(call->return_);
    }

    if(call->return2_){
        manager.setLocation(call->return2_, ltac::Register(descriptor->int_return_register2()));
        manager.set_written(call->return2_);
    }

    manager.restore_pushed_registers();
}

void ltac::StatementCompiler::compile_ASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_reg_no_move(quadruple->result);
    
    //Copy it in the register
    manager.copy(*quadruple->arg1, reg);

    //The variable has been written
    manager.set_written(quadruple->result);

    //If the address of the variable is escaped, we have to spill its value directly
    if(manager.is_escaped(quadruple->result)){
        manager.spills(reg);
    }
}

void ltac::StatementCompiler::compile_PASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
        if((*ptr)->type()->is_pointer()){
            compile_ASSIGN(quadruple);
        } else {
            auto result_reg = manager.get_reg_no_move(quadruple->result);
            auto value_reg = register_guard<ltac::Register>(get_address_in_reg(*ptr, 0), manager);
            ltac::add_instruction(function, ltac::Operator::MOV, result_reg, value_reg);

            manager.set_written(quadruple->result);

            //If the address of the variable is escaped, we have to spill its value directly
            if(manager.is_escaped(quadruple->result)){
                manager.spills(result_reg);
            }
        }
    } else {
        compile_ASSIGN(quadruple);
    }
}

void ltac::StatementCompiler::compile_FASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_float_reg_no_move(quadruple->result);
    manager.copy(*quadruple->arg1, reg);

    manager.set_written(quadruple->result);

    //If the address of the variable is escaped, we have to spill its value directly
    if(manager.is_escaped(quadruple->result)){
        manager.spills(reg);
    }
}

void ltac::StatementCompiler::compile_ADD(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result = quadruple->result;

    //Optimize the special form a = a + b by using only one ADD instruction
    if(*quadruple->arg1 == result){
        auto reg = manager.get_reg(quadruple->result);
        ltac::add_instruction(function, ltac::Operator::ADD, reg, to_arg(*quadruple->arg2));
    } 
    //Optimize the special form a = b + a by using only one ADD instruction
    else if(*quadruple->arg2 == result){
        auto reg = manager.get_reg(quadruple->result);
        ltac::add_instruction(function, ltac::Operator::ADD, reg, to_arg(*quadruple->arg2));
    } 
    //In the other cases, use lea to perform the addition
    else {
        auto reg = manager.get_reg_no_move(quadruple->result);

        if(ltac::is_variable(*quadruple->arg1)){
            if(ltac::is_variable(*quadruple->arg2)){
                ltac::add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(to_register(ltac::get_variable(*quadruple->arg1)), to_register(ltac::get_variable(*quadruple->arg2))));
            } else {
                ltac::add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(to_register(ltac::get_variable(*quadruple->arg1)), boost::get<int>(*quadruple->arg2)));
            }
        } else {
            if(ltac::is_variable(*quadruple->arg2)){
                ltac::add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(boost::get<int>(*quadruple->arg1)), manager.get_reg(ltac::get_variable(*quadruple->arg2)));
            } else {
                ltac::add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(boost::get<int>(*quadruple->arg1)), boost::get<int>(*quadruple->arg2));
            }
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_SUB(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result = quadruple->result;

    //Optimize the special form a = a - b by using only one SUB instruction
    if(*quadruple->arg1 == result){
        auto reg = manager.get_reg(quadruple->result);
        ltac::add_instruction(function, ltac::Operator::SUB, reg, to_arg(*quadruple->arg2));
    } 
    //In the other cases, move the first arg into the result register and then subtract the second arg into it
    else {
        auto reg = manager.get_reg_no_move(quadruple->result);
        ltac::add_instruction(function, ltac::Operator::MOV, reg, to_arg(*quadruple->arg1));
        ltac::add_instruction(function, ltac::Operator::SUB, reg, to_arg(*quadruple->arg2));
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_MUL(std::shared_ptr<mtac::Quadruple> quadruple){
    //This case should never happen unless the optimizer has bugs
    assert(!(isInt(*quadruple->arg1) && isInt(*quadruple->arg2)));

    //Form  x = x * y
    if(*quadruple->arg1 == quadruple->result){
        auto reg = manager.get_reg(quadruple->result);
        ltac::add_instruction(function, ltac::Operator::MUL, reg, to_arg(*quadruple->arg2));
    }
    //Form x = y * x
    else if(*quadruple->arg2 == quadruple->result){
        auto reg = manager.get_reg(quadruple->result);
        ltac::add_instruction(function, ltac::Operator::MUL, reg, to_arg(*quadruple->arg1));
    }
    //Form x = y * z (z: immediate)
    else if(isVariable(*quadruple->arg1) && isInt(*quadruple->arg2)){
        ltac::add_instruction(function, ltac::Operator::MUL, manager.get_reg_no_move(quadruple->result), to_arg(*quadruple->arg1), to_arg(*quadruple->arg2));
    }
    //Form x = y * z (y: immediate)
    else if(isInt(*quadruple->arg1) && isVariable(*quadruple->arg2)){
        ltac::add_instruction(function, ltac::Operator::MUL, manager.get_reg_no_move(quadruple->result), to_arg(*quadruple->arg2), to_arg(*quadruple->arg1));
    }
    //Form x = y * z (both variables)
    else if(isVariable(*quadruple->arg1) && isVariable(*quadruple->arg2)){
        auto reg = manager.get_reg_no_move(quadruple->result);
        manager.copy(*quadruple->arg1, reg);
        ltac::add_instruction(function, ltac::Operator::MUL, reg, to_arg(*quadruple->arg2));
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_DIV(std::shared_ptr<mtac::Quadruple> quadruple){
    //This optimization cannot be done in the peephole optimizer
    //Form x = x / y when y is power of two
    if(*quadruple->arg1 == quadruple->result && isInt(*quadruple->arg2)){
        int constant = boost::get<int>(*quadruple->arg2);

        if(isPowerOfTwo(constant)){
            ltac::add_instruction(function, ltac::Operator::SHIFT_RIGHT, manager.get_reg(quadruple->result), powerOfTwo(constant));

            manager.set_written(quadruple->result);

            return;
        }
    }

    manager.spills(ltac::Register(descriptor->d_register()));
    manager.reserve(ltac::Register(descriptor->d_register()));

    //Form x = x / y
    if(*quadruple->arg1 == quadruple->result){
        manager.safe_move(quadruple->result, ltac::Register(descriptor->a_register()));

        div_eax(quadruple);

    } 
    //Form x = y / z (y: variable)
    else if(ltac::is_variable(*quadruple->arg1)){
        auto A = ltac::Register(descriptor->a_register());

        manager.spills(A);
        manager.reserve(A);

        manager.copy(ltac::get_variable(*quadruple->arg1), A);

        div_eax(quadruple);

        manager.release(A);
        manager.setLocation(quadruple->result, A);
    } else {
        manager.spills(ltac::Register(descriptor->a_register()));
        manager.reserve(ltac::Register(descriptor->a_register()));

        manager.copy(*quadruple->arg1, ltac::Register(descriptor->a_register()));

        div_eax(quadruple);

        manager.release(ltac::Register(descriptor->a_register()));
        manager.setLocation(quadruple->result, ltac::Register(descriptor->a_register()));
    }

    manager.release(ltac::Register(descriptor->d_register()));

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_MOD(std::shared_ptr<mtac::Quadruple> quadruple){
    manager.spills(ltac::Register(descriptor->a_register()));
    manager.spills(ltac::Register(descriptor->d_register()));

    manager.reserve(ltac::Register(descriptor->a_register()));
    manager.reserve(ltac::Register(descriptor->d_register()));

    manager.copy(*quadruple->arg1, ltac::Register(descriptor->a_register()));

    div_eax(quadruple);

    //result is in edx (no need to move it now)
    manager.setLocation(quadruple->result, ltac::Register(descriptor->d_register()));

    manager.release(ltac::Register(descriptor->a_register()));

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FADD(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result = quadruple->result;

    //Optimize the special form a = a + b
    if(*quadruple->arg1 == result){
        auto reg = manager.get_float_reg(result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FADD, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg2));
        }
    }
    //Optimize the special form a = b + a by using only one instruction
    else if(*quadruple->arg2 == result){
        auto reg = manager.get_float_reg(result);

        if(mtac::isFloat(*quadruple->arg1)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg1, reg2);
            ltac::add_instruction(function, ltac::Operator::FADD, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg1));
        }
    }
    //In the other forms, use two instructions
    else {
        auto reg = manager.get_float_reg_no_move(result);
        manager.copy(*quadruple->arg1, reg);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FADD, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg2));
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FSUB(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result = quadruple->result;

    //Optimize the special form a = a - b
    if(*quadruple->arg1 == result){
        auto reg = manager.get_float_reg(result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FSUB, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FSUB, reg, to_arg(*quadruple->arg2));
        }
    } else {
        auto reg = manager.get_float_reg_no_move(result);
        manager.copy(*quadruple->arg1, reg);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FSUB, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FSUB, reg, to_arg(*quadruple->arg2));
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FMUL(std::shared_ptr<mtac::Quadruple> quadruple){
    //Form  x = x * y
    if(*quadruple->arg1 == quadruple->result){
        auto reg = manager.get_float_reg(quadruple->result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FMUL, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
        }
    }
    //Form x = y * x
    else if(*quadruple->arg2 == quadruple->result){
        auto reg = manager.get_float_reg(quadruple->result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FMUL, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
        }
    } 
    //General form
    else  {
        auto reg = manager.get_float_reg_no_move(quadruple->result);
        manager.copy(*quadruple->arg1, reg);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FMUL, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FDIV(std::shared_ptr<mtac::Quadruple> quadruple){
    //Form x = x / y
    if(*quadruple->arg1 == quadruple->result){
        auto reg = manager.get_float_reg(quadruple->result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FDIV, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FDIV, reg, to_arg(*quadruple->arg2));
        }
    } 
    //General form
    else {
        auto reg = manager.get_float_reg_no_move(quadruple->result);
        manager.copy(*quadruple->arg1, reg);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(function, ltac::Operator::FDIV, reg, reg2);
        } else {
            ltac::add_instruction(function, ltac::Operator::FDIV, reg, to_arg(*quadruple->arg2));
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_EQUALS(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVE, quadruple);
}

void ltac::StatementCompiler::compile_NOT_EQUALS(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVNE, quadruple);
}

void ltac::StatementCompiler::compile_GREATER(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVG, quadruple);
}

void ltac::StatementCompiler::compile_GREATER_EQUALS(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVGE, quadruple);
}

void ltac::StatementCompiler::compile_LESS(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVL, quadruple);
}

void ltac::StatementCompiler::compile_LESS_EQUALS(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVLE, quadruple);
}

void ltac::StatementCompiler::compile_FE(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVE, quadruple);
}

void ltac::StatementCompiler::compile_FNE(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVNE, quadruple);
}

void ltac::StatementCompiler::compile_FG(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVA, quadruple);
}

void ltac::StatementCompiler::compile_FGE(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVAE, quadruple);
}

void ltac::StatementCompiler::compile_FLE(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVBE, quadruple);
}

void ltac::StatementCompiler::compile_FL(std::shared_ptr<mtac::Quadruple> quadruple){
    set_if_cc(ltac::Operator::CMOVB, quadruple);
}

void ltac::StatementCompiler::compile_MINUS(std::shared_ptr<mtac::Quadruple> quadruple){
    //Constants should have been replaced by the optimizer
    assert(isVariable(*quadruple->arg1));

    ltac::add_instruction(function, ltac::Operator::NEG, manager.get_reg(ltac::get_variable(*quadruple->arg1)));

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FMINUS(std::shared_ptr<mtac::Quadruple> quadruple){
    //Constants should have been replaced by the optimizer
    assert(isVariable(*quadruple->arg1));

    auto reg = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
    manager.copy(-1.0, reg);

    ltac::add_instruction(function, ltac::Operator::FMUL, manager.get_float_reg(ltac::get_variable(*quadruple->arg1)), reg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_I2F(std::shared_ptr<mtac::Quadruple> quadruple){
    //Constants should have been replaced by the optimizer
    assert(isVariable(*quadruple->arg1));

    auto reg = manager.get_reg(ltac::get_variable(*quadruple->arg1));
    auto resultReg = manager.get_float_reg_no_move(quadruple->result);

    ltac::add_instruction(function, ltac::Operator::I2F, resultReg, reg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_F2I(std::shared_ptr<mtac::Quadruple> quadruple){
    //Constants should have been replaced by the optimizer
    assert(isVariable(*quadruple->arg1));

    auto reg = manager.get_float_reg(ltac::get_variable(*quadruple->arg1));
    auto resultReg = manager.get_reg_no_move(quadruple->result);

    ltac::add_instruction(function, ltac::Operator::F2I, resultReg, reg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_DOT(std::shared_ptr<mtac::Quadruple> quadruple){
    std::shared_ptr<ltac::Instruction> instruction;
    
    if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
        auto variable = *var_ptr;

        if(variable->type()->is_pointer()){
            assert(boost::get<int>(&*quadruple->arg2));
            int offset = boost::get<int>(*quadruple->arg2);

            auto reg = manager.get_reg_no_move(quadruple->result);
            instruction = ltac::add_instruction(function, ltac::Operator::MOV, reg, to_pointer(variable, offset));
        } else {
            if(ltac::is_float_var(quadruple->result)){
                auto reg = manager.get_float_reg_no_move(quadruple->result);
                instruction = ltac::add_instruction(function, ltac::Operator::FMOV, reg, to_address(variable, *quadruple->arg2));
            } else {
                auto reg = manager.get_reg_no_move(quadruple->result);
                instruction = ltac::add_instruction(function, ltac::Operator::MOV, reg, to_address(variable, *quadruple->arg2));
            }
        }
    } else if(auto* string_ptr = boost::get<std::string>(&*quadruple->arg1)){
        auto reg = manager.get_reg_no_move(quadruple->result);

        if(auto* offset_ptr = boost::get<int>(&*quadruple->arg2)){
            instruction = ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address(*string_ptr, *offset_ptr));
        } else if(auto* offset_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
            auto offset_reg = manager.get_reg(*offset_ptr);
            instruction = ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address(*string_ptr, offset_reg));
        }
    }

    switch(quadruple->size){
        case mtac::Size::BYTE:
            instruction->size = ltac::Size::BYTE;
            break;
        case mtac::Size::WORD:
            instruction->size = ltac::Size::WORD;
            break;
        case mtac::Size::DOUBLE_WORD:
            instruction->size = ltac::Size::DOUBLE_WORD;
            break;
        case mtac::Size::QUAD_WORD:
            instruction->size = ltac::Size::QUAD_WORD;
            break;
        default:
            instruction->size = ltac::Size::DEFAULT;
            break;
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FDOT(std::shared_ptr<mtac::Quadruple> quadruple){
    assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));
    auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

    assert(boost::get<int>(&*quadruple->arg2));
    int offset = boost::get<int>(*quadruple->arg2);

    auto reg = manager.get_float_reg_no_move(quadruple->result);

    if(variable->type()->is_pointer()){
        ltac::add_instruction(function, ltac::Operator::FMOV, reg, to_pointer(variable, offset));
    } else {
        ltac::add_instruction(function, ltac::Operator::FMOV, reg, to_address(variable, offset));
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_PDOT(std::shared_ptr<mtac::Quadruple> quadruple){
    assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));
    auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

    auto reg = manager.get_reg_no_move(quadruple->result);

    if(mtac::is<int>(*quadruple->arg2)){
        int offset = boost::get<int>(*quadruple->arg2);

        auto reg2 = register_guard<ltac::Register>(get_address_in_reg(variable, offset), manager);

        ltac::add_instruction(function, ltac::Operator::MOV, reg, reg2);
    } else {
        assert(ltac::is_variable(*quadruple->arg2));

        auto offset = manager.get_reg(ltac::get_variable(*quadruple->arg2));
        auto reg2 = register_guard<ltac::Register>(get_address_in_reg2(variable, offset), manager);

        ltac::add_instruction(function, ltac::Operator::MOV, reg, reg2);
    }
                
    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_DOT_ASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    if(quadruple->result->type()->is_pointer()){
        ASSERT(boost::get<int>(&*quadruple->arg1), "The offset must be be an int");
        int offset = boost::get<int>(*quadruple->arg1);
        ltac::add_instruction(function, ltac::Operator::MOV, to_pointer(quadruple->result, offset), to_arg(*quadruple->arg2));
    } else {
        ltac::add_instruction(function, ltac::Operator::MOV, to_address(quadruple->result, *quadruple->arg1), to_arg(*quadruple->arg2));
    }
}

void ltac::StatementCompiler::compile_DOT_FASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = register_guard<ltac::FloatRegister>(manager.get_free_float_reg(), manager);
    manager.copy(*quadruple->arg2, reg);

    if(quadruple->result->type()->is_pointer()){
        ASSERT(boost::get<int>(&*quadruple->arg1), "The offset must be be an int");
        int offset = boost::get<int>(*quadruple->arg1);
        ltac::add_instruction(function, ltac::Operator::FMOV, to_pointer(quadruple->result, offset), reg);
    } else {
        ltac::add_instruction(function, ltac::Operator::FMOV, to_address(quadruple->result, *quadruple->arg1), reg);
    }
}

void ltac::StatementCompiler::compile_DOT_PASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    ASSERT(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2), "Can only take the address of a variable");
    auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg2); 

    auto reg = register_guard<ltac::Register>(get_address_in_reg(variable, 0), manager);
    ltac::add_instruction(function, ltac::Operator::MOV, to_address(quadruple->result, *quadruple->arg1), reg); 
}

void ltac::StatementCompiler::compile_NOT(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_reg_no_move(quadruple->result);
    manager.copy(*quadruple->arg1, reg);
    ltac::add_instruction(function, ltac::Operator::NOT, reg); 
}

void ltac::StatementCompiler::compile_AND(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_reg_no_move(quadruple->result);
    manager.copy(*quadruple->arg1, reg);
    ltac::add_instruction(function, ltac::Operator::AND, reg, boost::get<int>(*quadruple->arg2));
}

void ltac::StatementCompiler::compile_RETURN(std::shared_ptr<mtac::Quadruple> quadruple){
    //A return without args is the same as exiting from the function
    if(quadruple->arg1){
        if(isFloat(*quadruple->arg1)){
            manager.spills(ltac::FloatRegister(descriptor->float_return_register()));
            manager.move(*quadruple->arg1, ltac::FloatRegister(descriptor->float_return_register()));
        } else if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1) && ltac::is_float_var(ltac::get_variable(*quadruple->arg1))){
            auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

            auto reg = manager.get_float_reg(variable);
            if(reg != ltac::FloatRegister(descriptor->float_return_register())){
                manager.spills(ltac::FloatRegister(descriptor->float_return_register()));
                ltac::add_instruction(function, ltac::Operator::FMOV, ltac::FloatRegister(descriptor->float_return_register()), reg);
            }
        } else {
            auto reg1 = ltac::Register(descriptor->int_return_register1());
            auto reg2 = ltac::Register(descriptor->int_return_register2());

            manager.spills_if_necessary(reg1, *quadruple->arg1);

            bool necessary = true;
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                if(manager.in_register(*ptr, reg1)){
                    necessary = false;
                }
            }    

            if(necessary){
                ltac::add_instruction(function, ltac::Operator::MOV, reg1, to_arg(*quadruple->arg1));
            }

            if(quadruple->arg2){
                manager.spills_if_necessary(reg2, *quadruple->arg2);

                necessary = true;
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                    if(manager.in_register(*ptr, reg2)){
                        necessary = false;
                    }
                }    

                if(necessary){
                    ltac::add_instruction(function, ltac::Operator::MOV, reg2, to_arg(*quadruple->arg2));
                }
            }
        }
    }

    ltac::add_instruction(function, ltac::Operator::ADD, ltac::SP, function->context->size());
    bp_offset -= function->context->size();

    //The basic block must be ended before the jump
    end_basic_block();

    if(!option_defined("fomit-frame-pointer")){
        ltac::add_instruction(function, ltac::Operator::LEAVE);
    }

    ltac::add_instruction(function, ltac::Operator::RET);
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Quadruple> quadruple){
    set_current(quadruple);

    switch(quadruple->op){
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
        case mtac::Operator::NOP:
            //No code necessary
            break;
    }
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::NoOp>){
    //Nothing to do
}

void ltac::StatementCompiler::operator()(std::string& str){
    function->add(str);

    if(offset_labels.find(str) != offset_labels.end()){
        bp_offset = offset_labels[str];
        offset_labels.erase(str);
    }
}
