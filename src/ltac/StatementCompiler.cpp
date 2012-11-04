//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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

#include "mtac/Statement.hpp"
#include "mtac/Utils.hpp" 

#include "ltac/StatementCompiler.hpp"
#include "ltac/Utils.hpp"
#include "ltac/Statement.hpp"

using namespace eddic;

namespace {

ltac::Address stack_address(int offset){
    return ltac::Address(ltac::BP, offset);
}

ltac::Address stack_address(ltac::AddressRegister offsetReg, int offset){
    return ltac::Address(ltac::BP, offsetReg, 1, offset);
}

} //end of anonymous namespace

ltac::StatementCompiler::StatementCompiler(const std::vector<ltac::Register>& registers, const std::vector<ltac::FloatRegister>& float_registers, 
        mtac::function_p function, std::shared_ptr<FloatPool> float_pool) : 
        manager(registers, float_registers, function, float_pool), function(function), float_pool(float_pool) {}

void ltac::StatementCompiler::set_current(mtac::Statement statement){
    manager.set_current(statement);
}

void ltac::StatementCompiler::reset(){
    manager.reset();
}

void ltac::StatementCompiler::collect_parameters(std::shared_ptr<eddic::Function> definition){
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

                    ltac::add_instruction(bb, ltac::Operator::MOV, reg, stack_address(position.offset()));

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

            ltac::add_instruction(bb, ltac::Operator::MOV, reg, stack_address(position.offset()));

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
    ltac::add_instruction(bb, ltac::Operator::MOV, reg, to_arg(argument));
    uses.push_back(reg);
}

void ltac::StatementCompiler::pass_in_float_register(mtac::Argument& argument, int position){
    auto reg = manager.get_bound_pseudo_float_reg(descriptor->float_param_register(position));

    if(auto* ptr = boost::get<int>(&argument)){
        auto label = float_pool->label(static_cast<double>(*ptr));
        ltac::add_instruction(bb, ltac::Operator::FMOV, reg, ltac::Address(label));
    } else if(auto* ptr = boost::get<double>(&argument)){
        auto label = float_pool->label(*ptr);
        ltac::add_instruction(bb, ltac::Operator::FMOV, reg, ltac::Address(label));
    } else {
        ltac::add_instruction(bb, ltac::Operator::FMOV, reg, to_arg(argument));
    }
    
    float_uses.push_back(reg);
}

void ltac::StatementCompiler::compare_binary(mtac::Argument& arg1, mtac::Argument& arg2){
    //The first argument is not important, it can be immediate, but the second must be a register
    if(auto* ptr = boost::get<int>(&arg1)){
        auto reg1 = manager.get_free_pseudo_reg();

        ltac::add_instruction(bb, ltac::Operator::MOV, reg1, *ptr);

        auto reg2 = manager.get_pseudo_reg(ltac::get_variable(arg2));

        ltac::add_instruction(bb, ltac::Operator::CMP_INT, reg1, reg2);
    } else {
        auto reg1 = manager.get_pseudo_reg(ltac::get_variable(arg1));
        auto reg2 = to_arg(arg2);

        ltac::add_instruction(bb, ltac::Operator::CMP_INT, reg1, reg2);
    }
}

void ltac::StatementCompiler::compare_float_binary(mtac::Argument& arg1, mtac::Argument& arg2){
    //Comparisons of constant should have been handled by the optimizer
    assert(!(isFloat(arg1) && isFloat(arg2))); 

    //If both args are variables
    if(isVariable(arg1) && isVariable(arg2)){
        auto reg1 = manager.get_pseudo_float_reg(ltac::get_variable(arg1));
        auto reg2 = manager.get_pseudo_float_reg(ltac::get_variable(arg2));

        ltac::add_instruction(bb, ltac::Operator::CMP_FLOAT, reg1, reg2);
    } else if(isVariable(arg1) && isFloat(arg2)){
        auto reg1 = manager.get_pseudo_float_reg(ltac::get_variable(arg1));
        auto reg2 = manager.get_free_pseudo_float_reg();

        manager.copy(arg2, reg2);

        ltac::add_instruction(bb, ltac::Operator::CMP_FLOAT, reg1, reg2);
    } else if(isFloat(arg1) && isVariable(arg2)){
        auto reg1 = manager.get_free_pseudo_float_reg();
        auto reg2 = manager.get_pseudo_float_reg(ltac::get_variable(arg2));

        manager.copy(arg1, reg1);

        ltac::add_instruction(bb, ltac::Operator::CMP_FLOAT, reg1, reg2);
    }
}

void ltac::StatementCompiler::compare_unary(mtac::Argument arg1){
    if(auto* ptr = boost::get<int>(&arg1)){
        auto reg = manager.get_free_pseudo_reg();

        ltac::add_instruction(bb, ltac::Operator::MOV, reg, *ptr);

        ltac::add_instruction(bb, ltac::Operator::OR, reg, reg);
    } else {
        auto reg = manager.get_pseudo_reg(ltac::get_variable(arg1));

        ltac::add_instruction(bb, ltac::Operator::OR, reg, reg);
    }
}

void ltac::StatementCompiler::set_if_cc(ltac::Operator set, std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_pseudo_reg_no_move(quadruple->result);

    //The default value is 0
    ltac::add_instruction(bb, ltac::Operator::MOV, reg, 0);

    //The first argument is not important, it can be immediate, but the second must be a register
    if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
        auto cmp_reg = manager.get_free_pseudo_reg();

        ltac::add_instruction(bb, ltac::Operator::MOV, cmp_reg, *ptr); 
        ltac::add_instruction(bb, ltac::Operator::CMP_INT, cmp_reg, to_arg(*quadruple->arg2)); 
    } else {
        ltac::add_instruction(bb, ltac::Operator::CMP_INT, to_arg(*quadruple->arg1), to_arg(*quadruple->arg2)); 
    }

    //Conditionally move 1 in the register
    auto value_reg = manager.get_free_pseudo_reg();
    ltac::add_instruction(bb, ltac::Operator::MOV, value_reg, 1); 
    ltac::add_instruction(bb, set, reg, value_reg); 

    manager.set_written(quadruple->result);
}
        
void ltac::StatementCompiler::push(ltac::Argument arg){
    ltac::add_instruction(bb, ltac::Operator::PUSH, arg);
}

void ltac::StatementCompiler::pop(ltac::Argument arg){
    ltac::add_instruction(bb, ltac::Operator::POP, arg);
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::IfFalse> if_false){
    manager.set_current(if_false);

    if(if_false->op){
        //Depending on the type of the operator, do a float or a int comparison
        if(ltac::is_float_operator(*if_false->op)){
            compare_float_binary(if_false->arg1, *if_false->arg2);

            switch(*if_false->op){
                case mtac::BinaryOperator::FE:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::NE));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FNE:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::E));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FL:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::AE));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FLE:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::A));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FG:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::BE));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FGE:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::B));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::P));
                    break;
                default:
                    ASSERT_PATH_NOT_TAKEN("This operation is not a float operator");
                    break;
            }
        } else {
            compare_binary(if_false->arg1, *if_false->arg2);

            switch(*if_false->op){
                case mtac::BinaryOperator::EQUALS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::NE));
                    break;
                case mtac::BinaryOperator::NOT_EQUALS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::E));
                    break;
                case mtac::BinaryOperator::LESS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::GE));
                    break;
                case mtac::BinaryOperator::LESS_EQUALS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::G));
                    break;
                case mtac::BinaryOperator::GREATER:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::LE));
                    break;
                case mtac::BinaryOperator::GREATER_EQUALS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::L));
                    break;
                default:
                    ASSERT_PATH_NOT_TAKEN("This operation is not a float operator");
                    break;
            }
        }
    } else {
        compare_unary(if_false->arg1);

        bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::Z));
    }
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::If> if_){
    manager.set_current(if_);

    if(if_->op){
        //Depending on the type of the operator, do a float or a int comparison
        if(ltac::is_float_operator(*if_->op)){
            compare_float_binary(if_->arg1, *if_->arg2);

            switch(*if_->op){
                case mtac::BinaryOperator::FE:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::E));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FNE:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::NE));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FL:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::B));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FLE:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::BE));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FG:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::A));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                case mtac::BinaryOperator::FGE:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::AE));
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::P));
                    break;
                default:
                    ASSERT_PATH_NOT_TAKEN("This operation is not a float operator");
                    break;
            }
        } else {
            compare_binary(if_->arg1, *if_->arg2);

            switch(*if_->op){
                case mtac::BinaryOperator::EQUALS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::E));
                    break;
                case mtac::BinaryOperator::NOT_EQUALS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::NE));
                    break;
                case mtac::BinaryOperator::LESS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::L));
                    break;
                case mtac::BinaryOperator::LESS_EQUALS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::LE));
                    break;
                case mtac::BinaryOperator::GREATER:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::G));
                    break;
                case mtac::BinaryOperator::GREATER_EQUALS:
                    bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::GE));
                    break;
                default:
                    ASSERT_PATH_NOT_TAKEN("This operation is not a float operator");
                    break;
            }
        }

    } else {
        compare_unary(if_->arg1);

        bb->l_statements.push_back(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::NZ));
    }
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Goto> goto_){
    manager.set_current(goto_);

    bb->l_statements.push_back(std::make_shared<ltac::Jump>(goto_->block->label, ltac::JumpType::ALWAYS));
}

ltac::PseudoRegister ltac::StatementCompiler::get_address_in_pseudo_reg(std::shared_ptr<Variable> var, int offset){
    auto reg = manager.get_free_pseudo_reg();

    ltac::add_instruction(bb, ltac::Operator::LEA, reg, address(var, offset));
    
    return reg;
}

ltac::PseudoRegister ltac::StatementCompiler::get_address_in_pseudo_reg2(std::shared_ptr<Variable> var, ltac::PseudoRegister offset){
    auto reg = manager.get_free_pseudo_reg();
    
    ltac::add_instruction(bb, ltac::Operator::LEA, reg, address(var, 0));
    ltac::add_instruction(bb, ltac::Operator::ADD, reg, offset);
    
    return reg;
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Param> param){
    manager.set_current(param);

    if(first_param){
        ltac::add_instruction(bb, ltac::Operator::PRE_PARAM);
        first_param = false;
    }
    
    std::shared_ptr<const Type> type;
    bool register_allocated = false;
    unsigned int position = 0;
        
    if(param->std_param.length() > 0 || (param->param && configuration->option_defined("fparameter-allocation"))){
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
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param->arg)){
            auto variable = *ptr;

            if(variable->type()->is_pointer()){
                auto reg = manager.get_pseudo_reg(variable);

                if(register_allocated){
                    auto param_reg = manager.get_bound_pseudo_reg(descriptor->int_param_register(position));
                    ltac::add_instruction(bb, ltac::Operator::MOV, param_reg, reg);
                    uses.push_back(param_reg);
                } else {
                    push(reg);
                }
            } else {
                auto reg = get_address_in_pseudo_reg(variable, 0);

                if(register_allocated){
                    auto param_reg = manager.get_bound_pseudo_reg(descriptor->int_param_register(position));
                    ltac::add_instruction(bb, ltac::Operator::MOV, param_reg, reg);
                    uses.push_back(param_reg);
                } else {
                    push(reg);
                }
            }
        } else {
            auto value = boost::get<int>(param->arg);
            push(value);
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
                auto reg1 = manager.get_free_pseudo_reg();
                auto reg2 = manager.get_pseudo_float_reg(*ptr);

                ltac::add_instruction(bb, ltac::Operator::MOV, reg1, reg2);
                push(reg1);
            } else {
                if((*ptr)->type()->is_array()){
                    auto position = (*ptr)->position();

                    if(position.isGlobal()){
                        auto reg = manager.get_free_pseudo_reg();

                        ltac::add_instruction(bb, ltac::Operator::MOV, reg, "V" + position.name());
                        push(reg);
                    } else if(position.isStack()){
                        auto reg = manager.get_free_pseudo_reg();

                        ltac::add_instruction(bb, ltac::Operator::LEA, reg, stack_address(position.offset()));
                        push(reg);
                    } else if(position.isParameter()){
                        push(stack_address(position.offset()));
                    }
                } else {
                    auto reg = manager.get_pseudo_reg(ltac::get_variable(param->arg));
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

mtac::function_p get_target(std::shared_ptr<mtac::Call> call, std::shared_ptr<mtac::Program> program){
    auto target_definition = call->functionDefinition;

    for(auto& function : program->functions){
        if(function->definition == target_definition){
            return function;
        }
    }

    return nullptr;
}

void ltac::StatementCompiler::operator()(std::shared_ptr<mtac::Call> call){
    manager.set_current(call);

    first_param = true;

    auto call_instruction = std::make_shared<ltac::Jump>(call->function, ltac::JumpType::CALL);
    call_instruction->target_function = get_target(call, program);
    call_instruction->uses = uses;
    call_instruction->float_uses = float_uses;
    bb->l_statements.push_back(call_instruction);

    uses.clear();
    float_uses.clear();

    int total = 0;

    unsigned int maxInt = descriptor->numberOfIntParamRegisters();
    unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();
    
    if(!call->functionDefinition->standard && !configuration->option_defined("fparameter-allocation")){
        maxInt = 0;
        maxFloat = 0;
    }

    for(auto& param : call->functionDefinition->parameters){
        auto type = param.paramType; 

        if(type->is_array()){
            //Passing an array is just passing an adress
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

    ltac::add_instruction(bb, ltac::Operator::ADD, ltac::SP, total);

    //The copies should be cleaned by the optimizations

    if(call->return_){
        if(call->return_->type() == FLOAT){
            auto reg = manager.get_pseudo_float_reg_no_move(call->return_);
            auto return_reg = manager.get_bound_pseudo_float_reg(descriptor->float_return_register());
            ltac::add_instruction(bb, ltac::Operator::FMOV, reg, return_reg);
        } else {
            auto reg = manager.get_pseudo_reg_no_move(call->return_);
            auto return_reg = manager.get_bound_pseudo_reg(descriptor->int_return_register1());
            ltac::add_instruction(bb, ltac::Operator::MOV, reg, return_reg);
        }

        manager.set_written(call->return_);
    }

    if(call->return2_){
        auto reg = manager.get_pseudo_reg_no_move(call->return2_);
        auto return_reg = manager.get_bound_pseudo_reg(descriptor->int_return_register2());
        ltac::add_instruction(bb, ltac::Operator::MOV, reg, return_reg);

        manager.set_written(call->return2_);
    }
}

void ltac::StatementCompiler::compile_ASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_pseudo_reg_no_move(quadruple->result);
    
    //Copy it in the register
    manager.copy(*quadruple->arg1, reg);

    //The variable has been written
    manager.set_written(quadruple->result);

    //If the address of the variable is escaped, we have to spill its value directly
    if(manager.is_escaped(quadruple->result)){
        //TODO manager.spills(reg);
    }
}

void ltac::StatementCompiler::compile_PASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
        if((*ptr)->type()->is_pointer()){
            compile_ASSIGN(quadruple);
        } else {
            auto result_reg = manager.get_pseudo_reg_no_move(quadruple->result);
            auto value_reg = get_address_in_pseudo_reg(*ptr, 0);
            ltac::add_instruction(bb, ltac::Operator::MOV, result_reg, value_reg);

            manager.set_written(quadruple->result);

            //If the address of the variable is escaped, we have to spill its value directly
            if(manager.is_escaped(quadruple->result)){
                //TODO manager.spills(result_reg);
            }
        }
    } else {
        compile_ASSIGN(quadruple);
    }
}

void ltac::StatementCompiler::compile_FASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_pseudo_float_reg_no_move(quadruple->result);
    manager.copy(*quadruple->arg1, reg);

    manager.set_written(quadruple->result);

    //If the address of the variable is escaped, we have to spill its value directly
    if(manager.is_escaped(quadruple->result)){
        //TODO manager.spills(reg);
    }
}

void ltac::StatementCompiler::compile_ADD(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result = quadruple->result;

    //Optimize the special form a = a + b by using only one ADD instruction
    if(*quadruple->arg1 == result){
        auto reg = manager.get_pseudo_reg(quadruple->result);
        ltac::add_instruction(bb, ltac::Operator::ADD, reg, to_arg(*quadruple->arg2));
    } 
    //Optimize the special form a = b + a by using only one ADD instruction
    else if(*quadruple->arg2 == result){
        auto reg = manager.get_pseudo_reg(quadruple->result);
        ltac::add_instruction(bb, ltac::Operator::ADD, reg, to_arg(*quadruple->arg2));
    } 
    //In the other cases, use lea to perform the addition
    else {
        auto reg = manager.get_pseudo_reg_no_move(quadruple->result);

        if(ltac::is_variable(*quadruple->arg1)){
            if(ltac::is_variable(*quadruple->arg2)){
                ltac::add_instruction(bb, ltac::Operator::LEA, reg, ltac::Address(
                            manager.get_pseudo_reg(ltac::get_variable(*quadruple->arg1)), 
                            manager.get_pseudo_reg(ltac::get_variable(*quadruple->arg2))));
            } else {
                ltac::add_instruction(bb, ltac::Operator::LEA, reg, ltac::Address(
                            manager.get_pseudo_reg(ltac::get_variable(*quadruple->arg1)), boost::get<int>(*quadruple->arg2)));
            }
        } else {
            if(ltac::is_variable(*quadruple->arg2)){
                ltac::add_instruction(bb, ltac::Operator::LEA, reg, 
                        ltac::Address(manager.get_pseudo_reg(ltac::get_variable(*quadruple->arg2)), boost::get<int>(*quadruple->arg1)));
            } else {
                ltac::add_instruction(bb, ltac::Operator::MOV, reg, boost::get<int>(*quadruple->arg1) + boost::get<int>(*quadruple->arg2));
            }
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_SUB(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result = quadruple->result;

    //Optimize the special form a = a - b by using only one SUB instruction
    if(*quadruple->arg1 == result){
        auto reg = manager.get_pseudo_reg(quadruple->result);
        ltac::add_instruction(bb, ltac::Operator::SUB, reg, to_arg(*quadruple->arg2));
    } 
    //In the other cases, move the first arg into the result register and then subtract the second arg into it
    else {
        auto reg = manager.get_pseudo_reg_no_move(quadruple->result);
        ltac::add_instruction(bb, ltac::Operator::MOV, reg, to_arg(*quadruple->arg1));
        ltac::add_instruction(bb, ltac::Operator::SUB, reg, to_arg(*quadruple->arg2));
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_MUL(std::shared_ptr<mtac::Quadruple> quadruple){
    //This case should never happen unless the optimizer has bugs
    assert(!(isInt(*quadruple->arg1) && isInt(*quadruple->arg2)));

    //Form  x = x * y
    if(*quadruple->arg1 == quadruple->result){
        auto reg = manager.get_pseudo_reg(quadruple->result);
        ltac::add_instruction(bb, ltac::Operator::MUL, reg, to_arg(*quadruple->arg2));
    }
    //Form x = y * x
    else if(*quadruple->arg2 == quadruple->result){
        auto reg = manager.get_pseudo_reg(quadruple->result);
        ltac::add_instruction(bb, ltac::Operator::MUL, reg, to_arg(*quadruple->arg1));
    }
    //Form x = y * z (z: immediate)
    else if(isVariable(*quadruple->arg1) && isInt(*quadruple->arg2)){
        ltac::add_instruction(bb, ltac::Operator::MUL, manager.get_pseudo_reg_no_move(quadruple->result), to_arg(*quadruple->arg1), to_arg(*quadruple->arg2));
    }
    //Form x = y * z (y: immediate)
    else if(isInt(*quadruple->arg1) && isVariable(*quadruple->arg2)){
        ltac::add_instruction(bb, ltac::Operator::MUL, manager.get_pseudo_reg_no_move(quadruple->result), to_arg(*quadruple->arg2), to_arg(*quadruple->arg1));
    }
    //Form x = y * z (both variables)
    else if(isVariable(*quadruple->arg1) && isVariable(*quadruple->arg2)){
        auto reg = manager.get_pseudo_reg_no_move(quadruple->result);
        manager.copy(*quadruple->arg1, reg);
        ltac::add_instruction(bb, ltac::Operator::MUL, reg, to_arg(*quadruple->arg2));
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_DIV(std::shared_ptr<mtac::Quadruple> quadruple){
    //This optimization cannot be done in the peephole optimizer
    //Form x = x / y when y is power of two
    if(*quadruple->arg1 == quadruple->result && isInt(*quadruple->arg2)){
        int constant = boost::get<int>(*quadruple->arg2);

        if(isPowerOfTwo(constant)){
            ltac::add_instruction(bb, ltac::Operator::SHIFT_RIGHT, manager.get_pseudo_reg(quadruple->result), powerOfTwo(constant));

            manager.set_written(quadruple->result);

            return;
        }
    }

    auto result_reg = manager.get_pseudo_reg_no_move(quadruple->result);
    auto a_reg = manager.get_bound_pseudo_reg(descriptor->a_register());
    auto d_reg = manager.get_bound_pseudo_reg(descriptor->d_register());

    manager.copy(*quadruple->arg1, a_reg);

    ltac::add_instruction(bb, ltac::Operator::MOV, d_reg, a_reg);
    ltac::add_instruction(bb, ltac::Operator::SHIFT_RIGHT, d_reg, static_cast<int>(INT->size(platform) * 8 - 1));

    if(isInt(*quadruple->arg2)){
        auto reg = manager.get_free_pseudo_reg();
        manager.move(*quadruple->arg2, reg);

        auto instruction = ltac::add_instruction(bb, ltac::Operator::DIV, reg);
        instruction->uses.push_back(a_reg);
        instruction->uses.push_back(d_reg);
    } else {
        auto instruction = ltac::add_instruction(bb, ltac::Operator::DIV, to_arg(*quadruple->arg2));
        instruction->uses.push_back(a_reg);
        instruction->uses.push_back(d_reg);
    }
    
    ltac::add_instruction(bb, ltac::Operator::MOV, result_reg, a_reg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_MOD(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result_reg = manager.get_pseudo_reg_no_move(quadruple->result);
    auto a_reg = manager.get_bound_pseudo_reg(descriptor->a_register());
    auto d_reg = manager.get_bound_pseudo_reg(descriptor->d_register());

    manager.copy(*quadruple->arg1, a_reg);

    ltac::add_instruction(bb, ltac::Operator::MOV, d_reg, a_reg);
    ltac::add_instruction(bb, ltac::Operator::SHIFT_RIGHT, d_reg, static_cast<int>(INT->size(platform) * 8 - 1));

    if(isInt(*quadruple->arg2)){
        auto reg = manager.get_free_pseudo_reg();
        manager.move(*quadruple->arg2, reg);

        auto instruction = ltac::add_instruction(bb, ltac::Operator::DIV, reg);
        instruction->uses.push_back(a_reg);
        instruction->uses.push_back(d_reg);
    } else {
        auto instruction = ltac::add_instruction(bb, ltac::Operator::DIV, to_arg(*quadruple->arg2));
        instruction->uses.push_back(a_reg);
        instruction->uses.push_back(d_reg);
    }

    ltac::add_instruction(bb, ltac::Operator::MOV, result_reg, d_reg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FADD(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result = quadruple->result;

    //Optimize the special form a = a + b
    if(*quadruple->arg1 == result){
        auto reg = manager.get_pseudo_float_reg(result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FADD, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FADD, reg, to_arg(*quadruple->arg2));
        }
    }
    //Optimize the special form a = b + a by using only one instruction
    else if(*quadruple->arg2 == result){
        auto reg = manager.get_pseudo_float_reg(result);

        if(mtac::isFloat(*quadruple->arg1)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg1, reg2);
            ltac::add_instruction(bb, ltac::Operator::FADD, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FADD, reg, to_arg(*quadruple->arg1));
        }
    }
    //In the other forms, use two instructions
    else {
        auto reg = manager.get_pseudo_float_reg_no_move(result);
        manager.copy(*quadruple->arg1, reg);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FADD, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FADD, reg, to_arg(*quadruple->arg2));
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FSUB(std::shared_ptr<mtac::Quadruple> quadruple){
    auto result = quadruple->result;

    //Optimize the special form a = a - b
    if(*quadruple->arg1 == result){
        auto reg = manager.get_pseudo_float_reg(result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FSUB, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FSUB, reg, to_arg(*quadruple->arg2));
        }
    } else {
        auto reg = manager.get_pseudo_float_reg_no_move(result);
        manager.copy(*quadruple->arg1, reg);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FSUB, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FSUB, reg, to_arg(*quadruple->arg2));
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FMUL(std::shared_ptr<mtac::Quadruple> quadruple){
    //Form  x = x * y
    if(*quadruple->arg1 == quadruple->result){
        auto reg = manager.get_pseudo_float_reg(quadruple->result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FMUL, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
        }
    }
    //Form x = y * x
    else if(*quadruple->arg2 == quadruple->result){
        auto reg = manager.get_pseudo_float_reg(quadruple->result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FMUL, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
        }
    } 
    //General form
    else  {
        auto reg = manager.get_pseudo_float_reg_no_move(quadruple->result);
        manager.copy(*quadruple->arg1, reg);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FMUL, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
        }
    }

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FDIV(std::shared_ptr<mtac::Quadruple> quadruple){
    //Form x = x / y
    if(*quadruple->arg1 == quadruple->result){
        auto reg = manager.get_pseudo_float_reg(quadruple->result);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FDIV, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FDIV, reg, to_arg(*quadruple->arg2));
        }
    } 
    //General form
    else {
        auto reg = manager.get_pseudo_float_reg_no_move(quadruple->result);
        manager.copy(*quadruple->arg1, reg);

        if(mtac::isFloat(*quadruple->arg2)){
            auto reg2 = manager.get_free_pseudo_float_reg();
            manager.copy(*quadruple->arg2, reg2);
            ltac::add_instruction(bb, ltac::Operator::FDIV, reg, reg2);
        } else {
            ltac::add_instruction(bb, ltac::Operator::FDIV, reg, to_arg(*quadruple->arg2));
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

    ltac::add_instruction(bb, ltac::Operator::NEG, manager.get_pseudo_reg(ltac::get_variable(*quadruple->arg1)));

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_FMINUS(std::shared_ptr<mtac::Quadruple> quadruple){
    //Constants should have been replaced by the optimizer
    assert(isVariable(*quadruple->arg1));

    auto reg = manager.get_free_pseudo_float_reg();
    manager.copy(-1.0, reg);

    ltac::add_instruction(bb, ltac::Operator::FMUL, manager.get_pseudo_float_reg(ltac::get_variable(*quadruple->arg1)), reg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_I2F(std::shared_ptr<mtac::Quadruple> quadruple){
    //Constants should have been replaced by the optimizer
    assert(isVariable(*quadruple->arg1));

    auto reg = manager.get_pseudo_reg(ltac::get_variable(*quadruple->arg1));
    auto result_reg = manager.get_pseudo_float_reg_no_move(quadruple->result);

    ltac::add_instruction(bb, ltac::Operator::I2F, result_reg, reg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_F2I(std::shared_ptr<mtac::Quadruple> quadruple){
    //Constants should have been replaced by the optimizer
    assert(isVariable(*quadruple->arg1));

    auto reg = manager.get_pseudo_float_reg(ltac::get_variable(*quadruple->arg1));
    auto result_reg = manager.get_pseudo_reg_no_move(quadruple->result);

    ltac::add_instruction(bb, ltac::Operator::F2I, result_reg, reg);

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_DOT(std::shared_ptr<mtac::Quadruple> quadruple){
    std::shared_ptr<ltac::Instruction> instruction;
    
    if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
        auto variable = *var_ptr;

        if(variable->type()->is_pointer() || (variable->type()->is_dynamic_array() && !variable->position().isParameter())){
            auto reg = manager.get_pseudo_reg_no_move(quadruple->result);
            instruction = ltac::add_instruction(bb, ltac::Operator::MOV, reg, address(variable, *quadruple->arg2));
        } else {
            if(ltac::is_float_var(quadruple->result)){
                auto reg = manager.get_pseudo_float_reg_no_move(quadruple->result);
                instruction = ltac::add_instruction(bb, ltac::Operator::FMOV, reg, address(variable, *quadruple->arg2));
            } else {
                auto reg = manager.get_pseudo_reg_no_move(quadruple->result);
                instruction = ltac::add_instruction(bb, ltac::Operator::MOV, reg, address(variable, *quadruple->arg2));
            }
        }
    } else if(auto* string_ptr = boost::get<std::string>(&*quadruple->arg1)){
        auto reg = manager.get_pseudo_reg_no_move(quadruple->result);

        if(auto* offset_ptr = boost::get<int>(&*quadruple->arg2)){
            instruction = ltac::add_instruction(bb, ltac::Operator::MOV, reg, ltac::Address(*string_ptr, *offset_ptr));
        } else if(auto* offset_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
            auto offset_reg = manager.get_pseudo_reg(*offset_ptr);
            instruction = ltac::add_instruction(bb, ltac::Operator::MOV, reg, ltac::Address(*string_ptr, offset_reg));
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

    auto reg = manager.get_pseudo_float_reg_no_move(quadruple->result);
    ltac::add_instruction(bb, ltac::Operator::FMOV, reg, address(variable, offset));

    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_PDOT(std::shared_ptr<mtac::Quadruple> quadruple){
    assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));
    auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

    auto reg = manager.get_pseudo_reg_no_move(quadruple->result);

    //TODO This should probably be done directly in get_address_in_pseudo_reg

    //The pointer has to be dereferenced
    if(variable->type()->is_pointer()){
        auto ptr_reg = manager.get_pseudo_reg(variable);

        if(mtac::is<int>(*quadruple->arg2)){
            int offset = boost::get<int>(*quadruple->arg2);
            ltac::add_instruction(bb, ltac::Operator::MOV, reg, ltac::Address(ptr_reg, offset));
        } else {
            assert(ltac::is_variable(*quadruple->arg2));

            auto offset = manager.get_pseudo_reg(ltac::get_variable(*quadruple->arg2));
            ltac::add_instruction(bb, ltac::Operator::MOV, reg, ltac::Address(ptr_reg, offset));
        }
    } else {
        if(mtac::is<int>(*quadruple->arg2)){
            int offset = boost::get<int>(*quadruple->arg2);

            auto reg2 = get_address_in_pseudo_reg(variable, offset);
            ltac::add_instruction(bb, ltac::Operator::MOV, reg, reg2);
        } else {
            assert(ltac::is_variable(*quadruple->arg2));

            auto offset = manager.get_pseudo_reg(ltac::get_variable(*quadruple->arg2));
            auto reg2 = get_address_in_pseudo_reg2(variable, offset);

            ltac::add_instruction(bb, ltac::Operator::MOV, reg, reg2);
        }
    }
    
    manager.set_written(quadruple->result);
}

void ltac::StatementCompiler::compile_DOT_ASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    ltac::add_instruction(bb, ltac::Operator::MOV, address(quadruple->result, *quadruple->arg1), to_arg(*quadruple->arg2));
}

void ltac::StatementCompiler::compile_DOT_FASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_free_pseudo_float_reg();
    manager.copy(*quadruple->arg2, reg);
    ltac::add_instruction(bb, ltac::Operator::FMOV, address(quadruple->result, *quadruple->arg1), reg);
}

void ltac::StatementCompiler::compile_DOT_PASSIGN(std::shared_ptr<mtac::Quadruple> quadruple){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
        auto variable = *ptr;

        auto reg = get_address_in_pseudo_reg(variable, 0);
        ltac::add_instruction(bb, ltac::Operator::MOV, address(quadruple->result, *quadruple->arg1), reg); 
        ltac::add_instruction(bb, ltac::Operator::MOV, address(quadruple->result, *quadruple->arg1), reg); 
    } else if(mtac::is<int>(*quadruple->arg2)){
        ltac::add_instruction(bb, ltac::Operator::MOV, address(quadruple->result, *quadruple->arg1), boost::get<int>(*quadruple->arg2)); 
    } else {
        ASSERT_PATH_NOT_TAKEN("Unsupported rhs type in DOT_PASSIGN");
    }
}

void ltac::StatementCompiler::compile_NOT(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_pseudo_reg_no_move(quadruple->result);
    manager.copy(*quadruple->arg1, reg);
    ltac::add_instruction(bb, ltac::Operator::NOT, reg); 
}

void ltac::StatementCompiler::compile_AND(std::shared_ptr<mtac::Quadruple> quadruple){
    auto reg = manager.get_pseudo_reg_no_move(quadruple->result);
    manager.copy(*quadruple->arg1, reg);
    ltac::add_instruction(bb, ltac::Operator::AND, reg, boost::get<int>(*quadruple->arg2));
}

void ltac::StatementCompiler::compile_RETURN(std::shared_ptr<mtac::Quadruple> quadruple){
    std::vector<ltac::PseudoRegister> uses;
    std::vector<ltac::PseudoFloatRegister> float_uses;

    //A return without args is the same as exiting from the function
    if(quadruple->arg1){
        if(isFloat(*quadruple->arg1)){
            auto return_reg = manager.get_bound_pseudo_float_reg(descriptor->float_return_register());
            manager.move(*quadruple->arg1, return_reg);
            float_uses.push_back(return_reg);
        } else if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1) && ltac::is_float_var(ltac::get_variable(*quadruple->arg1))){
            auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

            auto reg = manager.get_pseudo_float_reg(variable);
            auto return_reg = manager.get_bound_pseudo_float_reg(descriptor->float_return_register());
            
            ltac::add_instruction(bb, ltac::Operator::FMOV, return_reg, reg);
            float_uses.push_back(return_reg);
        } else {
            auto reg1 = manager.get_bound_pseudo_reg(descriptor->int_return_register1());
            ltac::add_instruction(bb, ltac::Operator::MOV, reg1, to_arg(*quadruple->arg1));
            uses.push_back(reg1);

            if(quadruple->arg2){
                auto reg2 = manager.get_bound_pseudo_reg(descriptor->int_return_register2());
                ltac::add_instruction(bb, ltac::Operator::MOV, reg2, to_arg(*quadruple->arg2));
                uses.push_back(reg2);
            }
        }
    }

    auto instruction = ltac::add_instruction(bb, ltac::Operator::PRE_RET);
    instruction->uses = uses;
    instruction->float_uses = float_uses;
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
    bb->l_statements.push_back(str);
}
