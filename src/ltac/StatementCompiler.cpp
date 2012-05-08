//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "FunctionContext.hpp"
#include "Utils.hpp"

#include "Labels.hpp"
#include "ltac/StatementCompiler.hpp"
#include "ltac/Utils.hpp"

#include "mtac/Utils.hpp" //TODO Perhaps part of this should be moved to ltac ? 

using namespace eddic;

//TODO Avoid as much as possible direct acess to the the registers fields of the manager

ltac::StatementCompiler::StatementCompiler(std::vector<ltac::Register> registers, std::vector<ltac::FloatRegister> float_registers, 
        std::shared_ptr<ltac::Function> function, std::shared_ptr<FloatPool> float_pool) : 
        function(function), manager(registers, float_registers, function, float_pool), float_pool(float_pool) {
    descriptor = getPlatformDescriptor(platform);
}

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

ltac::Register ltac::StatementCompiler::to_register(std::shared_ptr<Variable> var){
    return ltac::to_register(var, manager);
}

ltac::Argument ltac::StatementCompiler::to_arg(mtac::Argument argument){
    return ltac::to_arg(argument, manager);
}

ltac::Address ltac::StatementCompiler::to_address(std::shared_ptr<Variable> var, int offset){
    auto position = var->position();

    assert(!position.isTemporary());

    if(position.isStack()){
        return ltac::Address(ltac::BP, -position.offset() + offset);
    } else if(position.isParameter()){
        //The case of array is special because only the address is passed, not the complete array
        if(var->type().isArray())
        {
            auto reg = manager.get_free_reg();

            ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, position.offset()));

            manager.release(reg);

            return ltac::Address(reg, offset);
        } 
        //In the other cases, the value is passed, so we can compute the offset directly
        else {
            return ltac::Address(ltac::BP, position.offset() + offset);
        }
    } else if(position.isGlobal()){
        return ltac::Address("V" + position.name(), offset);
    } 

    ASSERT_PATH_NOT_TAKEN("Should never get there");
}

ltac::Address ltac::StatementCompiler::to_address(std::shared_ptr<Variable> var, mtac::Argument offset){
    if(auto* ptr = boost::get<int>(&offset)){
        return to_address(var, *ptr);
    }

    assert(ltac::is_variable(offset));

    auto position = var->position();
    assert(!position.isTemporary());

    auto offsetReg = manager.get_reg(ltac::get_variable(offset));

    if(position.isStack()){
        return ltac::Address(ltac::BP, offsetReg, 1, -1 * position.offset());
    } else if(position.isParameter()){
        auto reg = manager.get_free_reg();

        ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::Address(ltac::BP, position.offset()));

        manager.release(reg);

        return ltac::Address(reg, offsetReg);
    } else if(position.isGlobal()){
        return ltac::Address("V" + position.name(), offsetReg);
    } 

    ASSERT_PATH_NOT_TAKEN("Should never get there");
}

void ltac::StatementCompiler::pass_in_int_register(mtac::Argument& argument, int position){
    ltac::add_instruction(function, ltac::Operator::MOV, ltac::Register(descriptor->int_param_register(position)), to_arg(argument));
}

void ltac::StatementCompiler::pass_in_float_register(mtac::Argument& argument, int position){
    if(auto* ptr = boost::get<double>(&argument)){
        auto label = float_pool->label(*ptr);
        ltac::add_instruction(function, ltac::Operator::FMOV, ltac::FloatRegister(descriptor->float_param_register(position)), ltac::Address(label));
    } else {
        ltac::add_instruction(function, ltac::Operator::FMOV, ltac::FloatRegister(descriptor->float_param_register(position)), to_arg(argument));
    }
}

void ltac::StatementCompiler::compare_binary(mtac::Argument& arg1, mtac::Argument& arg2){
    //The first argument is not important, it can be immediate, but the second must be a register
    if(auto* ptr = boost::get<int>(&arg1)){
        auto reg1 = manager.get_free_reg();

        ltac::add_instruction(function, ltac::Operator::MOV, reg1, *ptr);

        auto reg2 = manager.get_reg(ltac::get_variable(arg2));

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::CMP_INT, reg1, reg2);

        manager.release(reg1);
    } else {
        auto reg1 = manager.get_reg(ltac::get_variable(arg1));

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::CMP_INT, reg1, to_arg(arg2));
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
        auto reg2 = manager.get_free_float_reg();

        manager.copy(arg2, reg2);

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::CMP_FLOAT, reg1, reg2);

        manager.release(reg2);
    } else if(isFloat(arg1) && isVariable(arg2)){
        auto reg1 = manager.get_free_float_reg();
        auto reg2 = manager.get_float_reg(ltac::get_variable(arg2));

        manager.copy(arg1, reg1);

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::CMP_FLOAT, reg1, reg2);

        manager.release(reg1);
    }
}

void ltac::StatementCompiler::compare_unary(mtac::Argument arg1){
    if(auto* ptr = boost::get<int>(&arg1)){
        auto reg = manager.get_free_reg();

        ltac::add_instruction(function, ltac::Operator::MOV, reg, *ptr);

        //The basic block must be ended before the jump
        end_basic_block();

        ltac::add_instruction(function, ltac::Operator::OR, reg, reg);

        manager.release(reg);
    } else {
        //The basic block must be ended before the jump
        end_basic_block();

        auto reg = manager.get_reg(ltac::get_variable(arg1));

        ltac::add_instruction(function, ltac::Operator::OR, reg, reg);
    }
}

void ltac::StatementCompiler::mul(std::shared_ptr<Variable> result, mtac::Argument& arg2){
    mtac::assertIntOrVariable(arg2);

    auto reg = manager.get_reg(result);
    ltac::add_instruction(function, ltac::Operator::MUL, reg, to_arg(arg2));
}

//Div eax by arg2 
void ltac::StatementCompiler::div_eax(std::shared_ptr<mtac::Quadruple> quadruple){
    ltac::add_instruction(function, ltac::Operator::MOV, ltac::Register(descriptor->d_register()), ltac::Register(descriptor->a_register()));
    ltac::add_instruction(function, ltac::Operator::SHIFT_RIGHT, ltac::Register(descriptor->d_register()), size(BaseType::INT) * 8 - 1);

    if(isInt(*quadruple->arg2)){
        auto reg = manager.get_free_reg();
        manager.move(*quadruple->arg2, reg);

        ltac::add_instruction(function, ltac::Operator::DIV, reg);

        if(manager.registers.reserved(reg)){
            manager.release(reg);
        }
    } else {
        ltac::add_instruction(function, ltac::Operator::DIV, to_arg(*quadruple->arg2));
    }
}

void ltac::StatementCompiler::div(std::shared_ptr<mtac::Quadruple> quadruple){
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
        manager.registers.setLocation(quadruple->result, A);
    } else {
        manager.spills(ltac::Register(descriptor->a_register()));
        manager.reserve(ltac::Register(descriptor->a_register()));

        manager.copy(*quadruple->arg1, ltac::Register(descriptor->a_register()));

        div_eax(quadruple);

        manager.release(ltac::Register(descriptor->a_register()));
        manager.registers.setLocation(quadruple->result, ltac::Register(descriptor->a_register()));
    }

    manager.release(ltac::Register(descriptor->d_register()));
}

void ltac::StatementCompiler::mod(std::shared_ptr<mtac::Quadruple> quadruple){
    manager.spills(ltac::Register(descriptor->a_register()));
    manager.spills(ltac::Register(descriptor->d_register()));

    manager.reserve(ltac::Register(descriptor->a_register()));
    manager.reserve(ltac::Register(descriptor->d_register()));

    manager.copy(*quadruple->arg1, ltac::Register(descriptor->a_register()));

    div_eax(quadruple);

    //result is in edx (no need to move it now)
    manager.registers.setLocation(quadruple->result, ltac::Register(descriptor->d_register()));

    manager.release(ltac::Register(descriptor->a_register()));
}

void ltac::StatementCompiler::set_if_cc(ltac::Operator set, std::shared_ptr<mtac::Quadruple>& quadruple){
    auto reg = manager.get_reg_no_move(quadruple->result);

    //The first argument is not important, it can be immediate, but the second must be a register
    if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
        auto reg = manager.get_free_reg();

        ltac::add_instruction(function, ltac::Operator::MOV, reg, *ptr); 
        ltac::add_instruction(function, ltac::Operator::CMP_INT, reg, to_arg(*quadruple->arg2)); 

        manager.release(reg);
    } else {
        ltac::add_instruction(function, ltac::Operator::CMP_INT, to_arg(*quadruple->arg1), to_arg(*quadruple->arg2)); 
    }

    //TODO Find a better way to achieve that
    auto valueReg = manager.get_free_reg();
    ltac::add_instruction(function, ltac::Operator::MOV, valueReg, 1); 
    ltac::add_instruction(function, set, reg, valueReg); 
    manager.release(valueReg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::IfFalse>& if_false){
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
                    assert(false && "This operation is not a float operator");
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
                    assert(false && "This operation is not a float operator");
                    break;
            }
        }
    } else {
        compare_unary(if_false->arg1);

        function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::Z));
    }
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::If>& if_){
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
                    assert(false && "This operation is not a float operator");
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
                    assert(false && "This operation is not a float operator");
                    break;
            }
        }

    } else {
        compare_unary(if_->arg1);

        function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::NZ));
    }
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Goto>& goto_){
    manager.set_current(goto_);

    //The basic block must be ended before the jump
    end_basic_block();

    function->add(std::make_shared<ltac::Jump>(goto_->block->label, ltac::JumpType::ALWAYS));
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Param>& param){
    manager.set_current(param);

    unsigned int maxInt = descriptor->numberOfIntParamRegisters();
    unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();

    manager.save_registers(param, descriptor);

    if(param->std_param.length() > 0 || param->param){
        boost::optional<Type> type;
        unsigned int position;

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

        if(*type == BaseType::INT && position <= maxInt){
            pass_in_int_register(param->arg, position);

            return;
        }

        if(*type == BaseType::FLOAT && position <= maxFloat){
            pass_in_float_register(param->arg, position);

            return;
        }
    } 

    //If the param as not been handled as register passing, push it on the stack 
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param->arg)){
        if(!(*ptr)->type().isArray() && ltac::is_float_var(*ptr)){
            auto reg1 = manager.get_free_reg();
            auto reg2 = manager.get_float_reg(*ptr);

            ltac::add_instruction(function, ltac::Operator::MOV, reg1, reg2);
            ltac::add_instruction(function, ltac::Operator::PUSH, reg1);

            manager.release(reg1);
        } else {
            if((*ptr)->type().isArray()){
                auto position = (*ptr)->position();

                if(position.isGlobal()){
                    auto reg = manager.get_free_reg();

                    auto offset = size((*ptr)->type().base()) * (*ptr)->type().size();

                    ltac::add_instruction(function, ltac::Operator::MOV, reg, "V" + position.name());
                    ltac::add_instruction(function, ltac::Operator::ADD, reg, static_cast<int>(offset));
                    ltac::add_instruction(function, ltac::Operator::PUSH, reg);

                    manager.release(reg);
                } else if(position.isStack()){
                    auto reg = manager.get_free_reg();

                    ltac::add_instruction(function, ltac::Operator::MOV, reg, ltac::BP);
                    ltac::add_instruction(function, ltac::Operator::ADD, reg, -position.offset());
                    ltac::add_instruction(function, ltac::Operator::PUSH, reg);

                    manager.release(reg);
                } else if(position.isParameter()){
                    ltac::add_instruction(function, ltac::Operator::PUSH, ltac::Address(ltac::BP, position.offset()));
                }
            } else {
                auto reg = manager.get_reg(ltac::get_variable(param->arg));
                ltac::add_instruction(function, ltac::Operator::PUSH, reg);
            }
        }
    } else if(auto* ptr = boost::get<double>(&param->arg)){
        auto label = float_pool->label(*ptr);
        ltac::add_instruction(function, ltac::Operator::PUSH, ltac::Address(label));
    } else {
        ltac::add_instruction(function, ltac::Operator::PUSH, to_arg(param->arg));
    }
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Call>& call){
    manager.set_current(call);

    function->add(std::make_shared<ltac::Jump>(call->function, ltac::JumpType::CALL));

    int total = 0;

    unsigned int maxInt = descriptor->numberOfIntParamRegisters();
    unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();

    for(auto& param : call->functionDefinition->parameters){
        Type type = param.paramType; 

        if(type.isArray()){
            //Passing an array is just passing an adress
            total += size(BaseType::INT);
        } else {
            if(type == BaseType::INT){
                //If the parameter is allocated in a register, there is no need to deallocate stack space for it
                if(maxInt > 0){
                    --maxInt;
                } else {
                    total += size(type);
                }
            } else if(type == BaseType::FLOAT){
                //If the parameter is allocated in a register, there is no need to deallocate stack space for it
                if(maxFloat > 0){
                    --maxFloat;
                } else {
                    total += size(type);
                }
            } else {
                total += size(type);
            }
        }
    }

    //TODO Do that in the peephole optimizer
    if(total > 0){
        ltac::add_instruction(function, ltac::Operator::FREE_STACK, total);
    }

    if(call->return_){
        if(call->return_->type() == BaseType::FLOAT){
            manager.float_registers.setLocation(call->return_, ltac::FloatRegister(descriptor->float_return_register()));
        } else {
            manager.registers.setLocation(call->return_, ltac::Register(descriptor->int_return_register1()));
        }

        manager.set_written(call->return_);
    }

    if(call->return2_){
        manager.registers.setLocation(call->return2_, ltac::Register(descriptor->int_return_register2()));
        manager.set_written(call->return2_);
    }

    manager.restore_pushed_registers();
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
    set_current(quadruple);

    switch(quadruple->op){
        case mtac::Operator::ASSIGN:
            {
                auto reg = manager.get_reg_no_move(quadruple->result);
                ltac::add_instruction(function, ltac::Operator::MOV, reg, to_arg(*quadruple->arg1));

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::FASSIGN:
            {
                auto reg = manager.get_float_reg_no_move(quadruple->result);
                manager.copy(*quadruple->arg1, reg);

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::ADD:
            {
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
                        if(ltac::is_variable(*quadruple->arg1)){
                            ltac::add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(boost::get<int>(*quadruple->arg1)), manager.get_reg(ltac::get_variable(*quadruple->arg2)));
                        } else {
                            ltac::add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(boost::get<int>(*quadruple->arg1)), boost::get<int>(*quadruple->arg2));
                        }
                    }
                }

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::SUB:
            {
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

                break;
            }
        case mtac::Operator::MUL:
            {
                //This case should never happen unless the optimized has bugs
                assert(!(isInt(*quadruple->arg1) && isInt(*quadruple->arg2)));

                //Form  x = x * y
                if(*quadruple->arg1 == quadruple->result){
                    mul(quadruple->result, *quadruple->arg2);
                }
                //Form x = y * x
                else if(*quadruple->arg2 == quadruple->result){
                    mul(quadruple->result, *quadruple->arg1);
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

                break;            
            }
        case mtac::Operator::DIV:
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

            div(quadruple);

            manager.set_written(quadruple->result);

            break;            
        case mtac::Operator::MOD:
            mod(quadruple);

            manager.set_written(quadruple->result);

            break;
        case mtac::Operator::FADD:
            {
                auto result = quadruple->result;

                //Optimize the special form a = a + b
                if(*quadruple->arg1 == result){
                    auto reg = manager.get_float_reg(result);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = manager.get_free_float_reg();
                        manager.copy(*quadruple->arg2, reg2);
                        ltac::add_instruction(function, ltac::Operator::FADD, reg, reg2);
                        manager.release(reg2);
                    } else {
                        ltac::add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg2));
                    }
                }
                //Optimize the special form a = b + a by using only one instruction
                else if(*quadruple->arg2 == result){
                    auto reg = manager.get_float_reg(result);

                    if(mtac::isFloat(*quadruple->arg1)){
                        auto reg2 = manager.get_free_float_reg();
                        manager.copy(*quadruple->arg1, reg2);
                        ltac::add_instruction(function, ltac::Operator::FADD, reg, reg2);
                        manager.release(reg2);
                    } else {
                        ltac::add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg1));
                    }
                }
                //In the other forms, use two instructions
                else {
                    auto reg = manager.get_float_reg_no_move(result);
                    manager.copy(*quadruple->arg1, reg);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = manager.get_free_float_reg();
                        manager.copy(*quadruple->arg2, reg2);
                        ltac::add_instruction(function, ltac::Operator::FADD, reg, reg2);
                        manager.release(reg2);
                    } else {
                        ltac::add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg2));
                    }
                }

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::FSUB:
            {
                auto result = quadruple->result;

                //Optimize the special form a = a - b
                if(*quadruple->arg1 == result){
                    auto reg = manager.get_float_reg(result);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = manager.get_free_float_reg();
                        manager.copy(*quadruple->arg2, reg2);
                        ltac::add_instruction(function, ltac::Operator::FSUB, reg, reg2);
                        manager.release(reg2);
                    } else {
                        ltac::add_instruction(function, ltac::Operator::FSUB, reg, to_arg(*quadruple->arg2));
                    }
                } else {
                    auto reg = manager.get_float_reg_no_move(result);
                    manager.copy(*quadruple->arg1, reg);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = manager.get_free_float_reg();
                        manager.copy(*quadruple->arg2, reg2);
                        ltac::add_instruction(function, ltac::Operator::FSUB, reg, reg2);
                        manager.release(reg2);
                    } else {
                        ltac::add_instruction(function, ltac::Operator::FSUB, reg, to_arg(*quadruple->arg2));
                    }
                }

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::FMUL:
            //Form  x = x * y
            if(*quadruple->arg1 == quadruple->result){
                auto reg = manager.get_float_reg(quadruple->result);

                if(mtac::isFloat(*quadruple->arg2)){
                    auto reg2 = manager.get_free_float_reg();
                    manager.copy(*quadruple->arg2, reg2);
                    ltac::add_instruction(function, ltac::Operator::FMUL, reg, reg2);
                    manager.release(reg2);
                } else {
                    ltac::add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
                }
            }
            //Form x = y * x
            else if(*quadruple->arg2 == quadruple->result){
                auto reg = manager.get_float_reg(quadruple->result);

                if(mtac::isFloat(*quadruple->arg2)){
                    auto reg2 = manager.get_free_float_reg();
                    manager.copy(*quadruple->arg2, reg2);
                    ltac::add_instruction(function, ltac::Operator::FMUL, reg, reg2);
                    manager.release(reg2);
                } else {
                    ltac::add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
                }
            } 
            //General form
            else  {
                auto reg = manager.get_float_reg_no_move(quadruple->result);
                manager.copy(*quadruple->arg1, reg);

                if(mtac::isFloat(*quadruple->arg2)){
                    auto reg2 = manager.get_free_float_reg();
                    manager.copy(*quadruple->arg2, reg2);
                    ltac::add_instruction(function, ltac::Operator::FMUL, reg, reg2);
                    manager.release(reg2);
                } else {
                    ltac::add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
                }
            }

            manager.set_written(quadruple->result);

            break;            
        case mtac::Operator::FDIV:
            //Form x = x / y
            if(*quadruple->arg1 == quadruple->result){
                auto reg = manager.get_float_reg(quadruple->result);

                if(mtac::isFloat(*quadruple->arg2)){
                    auto reg2 = manager.get_free_float_reg();
                    manager.copy(*quadruple->arg2, reg2);
                    ltac::add_instruction(function, ltac::Operator::FDIV, reg, reg2);
                    manager.release(reg2);
                } else {
                    ltac::add_instruction(function, ltac::Operator::FDIV, reg, to_arg(*quadruple->arg2));
                }
            } 
            //General form
            else {
                auto reg = manager.get_float_reg_no_move(quadruple->result);
                manager.copy(*quadruple->arg1, reg);

                if(mtac::isFloat(*quadruple->arg2)){
                    auto reg2 = manager.get_free_float_reg();
                    manager.copy(*quadruple->arg2, reg2);
                    ltac::add_instruction(function, ltac::Operator::FDIV, reg, reg2);
                    manager.release(reg2);
                } else {
                    ltac::add_instruction(function, ltac::Operator::FDIV, reg, to_arg(*quadruple->arg2));
                }
            }

            manager.set_written(quadruple->result);

            break;            
        case mtac::Operator::I2F:
            {
                //Constants should have been replaced by the optimizer
                assert(isVariable(*quadruple->arg1));

                auto reg = manager.get_reg(ltac::get_variable(*quadruple->arg1));
                auto resultReg = manager.get_float_reg_no_move(quadruple->result);

                ltac::add_instruction(function, ltac::Operator::I2F, resultReg, reg);

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::F2I:
            {
                //Constants should have been replaced by the optimizer
                assert(isVariable(*quadruple->arg1));

                auto reg = manager.get_float_reg(ltac::get_variable(*quadruple->arg1));
                auto resultReg = manager.get_reg_no_move(quadruple->result);

                ltac::add_instruction(function, ltac::Operator::F2I, resultReg, reg);

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::MINUS:
            {
                //Constants should have been replaced by the optimizer
                assert(isVariable(*quadruple->arg1));

                ltac::add_instruction(function, ltac::Operator::NEG, manager.get_reg(ltac::get_variable(*quadruple->arg1)));

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::FMINUS:
            {
                //Constants should have been replaced by the optimizer
                assert(isVariable(*quadruple->arg1));

                auto reg = manager.get_free_float_reg();
                manager.copy(-1.0, reg);

                ltac::add_instruction(function, ltac::Operator::FMUL, manager.get_float_reg(ltac::get_variable(*quadruple->arg1)), reg);

                manager.release(reg);

                manager.set_written(quadruple->result);

                break;    
            }
        case mtac::Operator::GREATER:
            set_if_cc(ltac::Operator::CMOVG, quadruple);
            break;
        case mtac::Operator::GREATER_EQUALS:
            set_if_cc(ltac::Operator::CMOVGE, quadruple);
            break;
        case mtac::Operator::LESS:
            set_if_cc(ltac::Operator::CMOVL, quadruple);
            break;
        case mtac::Operator::LESS_EQUALS:
            set_if_cc(ltac::Operator::CMOVLE, quadruple);
            break;
        case mtac::Operator::EQUALS:
            set_if_cc(ltac::Operator::CMOVE, quadruple);
            break;
        case mtac::Operator::NOT_EQUALS:
            set_if_cc(ltac::Operator::CMOVNE, quadruple);
            break;
        case mtac::Operator::FG:
            set_if_cc(ltac::Operator::CMOVA, quadruple);
            break;
        case mtac::Operator::FGE:
            set_if_cc(ltac::Operator::CMOVAE, quadruple);
            break;
        case mtac::Operator::FL:
            set_if_cc(ltac::Operator::CMOVB, quadruple);
            break;
        case mtac::Operator::FLE:
            set_if_cc(ltac::Operator::CMOVBE, quadruple);
            break;
        case mtac::Operator::FE:
            set_if_cc(ltac::Operator::CMOVE, quadruple);
            break;
        case mtac::Operator::FNE:
            set_if_cc(ltac::Operator::CMOVNE, quadruple);
            break;
        case mtac::Operator::DOT:
            {
                assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));
                assert(boost::get<int>(&*quadruple->arg2));

                auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
                int offset = boost::get<int>(*quadruple->arg2);

                auto reg = manager.get_reg_no_move(quadruple->result);
                ltac::add_instruction(function, ltac::Operator::MOV, reg, to_address(variable, offset));

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::FDOT:
            {
                assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));
                assert(boost::get<int>(&*quadruple->arg2));

                auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
                int offset = boost::get<int>(*quadruple->arg2);

                auto reg = manager.get_float_reg_no_move(quadruple->result);
                ltac::add_instruction(function, ltac::Operator::FMOV, reg, to_address(variable, offset));

                manager.set_written(quadruple->result);

                break;
            }
        case mtac::Operator::DOT_ASSIGN:
            {
                ASSERT(boost::get<int>(&*quadruple->arg1), "The offset must be be an int");

                int offset = boost::get<int>(*quadruple->arg1);

                ltac::add_instruction(function, ltac::Operator::MOV, to_address(quadruple->result, offset), to_arg(*quadruple->arg2));

                break;
            }
        case mtac::Operator::DOT_FASSIGN:
            {
                ASSERT(boost::get<int>(&*quadruple->arg1), "The offset must be be an int");

                int offset = boost::get<int>(*quadruple->arg1);
                auto reg = manager.get_free_float_reg();
                manager.copy(*quadruple->arg2, reg);

                ltac::add_instruction(function, ltac::Operator::FMOV, to_address(quadruple->result, offset), reg);

                manager.release(reg);

                break;
            }
        case mtac::Operator::ARRAY:
            {
                assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));

                if(ltac::is_float_var(quadruple->result)){
                    auto reg = manager.get_float_reg_no_move(quadruple->result);

                    ltac::add_instruction(function, ltac::Operator::FMOV, reg, to_address(ltac::get_variable(*quadruple->arg1), *quadruple->arg2));
                } else {
                    auto reg = manager.get_reg_no_move(quadruple->result);

                    ltac::add_instruction(function, ltac::Operator::MOV, reg, to_address(ltac::get_variable(*quadruple->arg1), *quadruple->arg2));
                }

                manager.set_written(quadruple->result);

                break;            
            }
        case mtac::Operator::ARRAY_ASSIGN:
            if(quadruple->result->type().base() == BaseType::FLOAT){
                auto reg = manager.get_free_float_reg();
                manager.copy(*quadruple->arg2, reg);

                ltac::add_instruction(function, ltac::Operator::FMOV, to_address(quadruple->result, *quadruple->arg1), reg);

                manager.release(reg);
            } else {
                ltac::add_instruction(function, ltac::Operator::MOV, to_address(quadruple->result, *quadruple->arg1), to_arg(*quadruple->arg2));
            }

            break;
        case mtac::Operator::RETURN:
            {
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
                            if(manager.registers.inRegister(*ptr, reg1)){
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
                                if(manager.registers.inRegister(*ptr, reg2)){
                                    necessary = false;
                                }
                            }    

                            if(necessary){
                                ltac::add_instruction(function, ltac::Operator::MOV, reg2, to_arg(*quadruple->arg2));
                            }
                        }
                    }
                }

                if(function->context->size() > 0){
                    ltac::add_instruction(function, ltac::Operator::FREE_STACK, function->context->size());
                }

                //The basic block must be ended before the jump
                end_basic_block();

                ltac::add_instruction(function, ltac::Operator::LEAVE);

                break;
            }
    }
}

void ltac::StatementCompiler::operator()(mtac::NoOp&){
    //Nothing to do
}

void ltac::StatementCompiler::operator()(std::string& str){
    function->add(str);
}
