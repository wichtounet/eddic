//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "FunctionContext.hpp"
#include "Labels.hpp"
#include "VisitorUtils.hpp"
#include "Platform.hpp"

#include "asm/Registers.hpp"

#include "ltac/Compiler.hpp"

#include "mtac/Utils.hpp" //TODO Perhaps part of this should be moved to ltac ? 

using namespace eddic;

void add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op){
    function->add(std::make_shared<ltac::Instruction>(op));
}

void add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1){
    function->add(std::make_shared<ltac::Instruction>(op, arg1));
}

void add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2){
    function->add(std::make_shared<ltac::Instruction>(op, arg1, arg2));
}

void add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2, ltac::Argument arg3){
    function->add(std::make_shared<ltac::Instruction>(arg3, arg1, op, arg2));
}

void ltac::Compiler::compile(std::shared_ptr<mtac::Program> source, std::shared_ptr<ltac::Program> target){
    for(auto& src_function : source->functions){
        auto target_function = std::make_shared<ltac::Function>(src_function->context, src_function->getName());

        target->functions.push_back(target_function);

        compile(src_function, target_function);
    }
}

void ltac::Compiler::compile(std::shared_ptr<mtac::Function> src_function, std::shared_ptr<ltac::Function> target_function){
    auto size = src_function->context->size();
    
    //Only if necessary, allocates size on the stack for the local variables
    if(size > 0){
        add_instruction(target_function, ltac::Operator::ALLOC_STACK, size);
    }
    
    auto iter = src_function->context->begin();
    auto end = src_function->context->end();

    for(; iter != end; iter++){
        auto var = iter->second;
        if(var->type().isArray() && var->position().isStack()){
            int position = -var->position().offset();

            add_instruction(target_function, ltac::Operator::MOV, ltac::Address(ltac::BP, position), var->type().size());

            if(var->type().base() == BaseType::INT){
                add_instruction(target_function, ltac::Operator::MEMSET, ltac::Address(ltac::BP, position, -8), var->type().size());
            } else if(var->type().base() == BaseType::STRING){
                add_instruction(target_function, ltac::Operator::MEMSET, ltac::Address(ltac::BP, position, -8), 2 * var->type().size());
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

    //Then we compile each of them
    for(auto block : src_function->getBasicBlocks()){
        compile(block, target_function);
    }

    //TODO Return optimization
    
    //Only if necessary, deallocates size on the stack for the local variables
    if(size > 0){
        add_instruction(target_function, ltac::Operator::FREE_STACK, size);
    }
}

namespace {

struct StatementCompiler : public boost::static_visitor<> {
    //The registers
    as::Registers<ltac::Register> registers;
    as::Registers<ltac::FloatRegister> float_registers;
    
    //Store the Register that are saved before call
    std::vector<ltac::Register> int_pushed;
    std::vector<ltac::FloatRegister> float_pushed;

    bool last = false;      //Is it the last basic block ?
    bool ended = false;     //Is the basic block ended ?

    //Allow to push needed register before the first push param
    bool first_param = true;

    //Reference to the statement for liveness
    mtac::Statement current;
    mtac::Statement next;

    //The function being compiled
    std::shared_ptr<ltac::Function> function;
    
    //Keep track of the written variables to spills them
    std::unordered_set<std::shared_ptr<Variable>> written;

    StatementCompiler(std::vector<ltac::Register> registers, std::vector<ltac::FloatRegister> float_registers, std::shared_ptr<ltac::Function> function) : 
        registers(registers, std::make_shared<Variable>("__fake_int__", newSimpleType(BaseType::INT), Position(PositionType::TEMPORARY))),
        float_registers(float_registers, std::make_shared<Variable>("__fake_float__", newSimpleType(BaseType::FLOAT), Position(PositionType::TEMPORARY))), 
        function(function) {
        //Nothing else to init
    }

    /* Register stuff  */
    
    ltac::Register get_free_reg(){
        //TODO
    }

    ltac::Register get_reg(std::shared_ptr<Variable> var){
        //TODO
    }
    
    ltac::Register get_reg_no_move(std::shared_ptr<Variable> var){
        //TODO
    }
    
    ltac::FloatRegister get_free_float_reg(){
        //TODO
    }

    ltac::FloatRegister get_float_reg(std::shared_ptr<Variable> var){
        //TODO
    }

    ltac::FloatRegister get_float_reg_no_move(std::shared_ptr<Variable> var){
        //TODO
    }

    void copy(mtac::Argument arg, ltac::Register reg){
        //TODO
    }
    
    void copy(mtac::Argument arg, ltac::FloatRegister reg){
        //TODO
    }
    
    void move(mtac::Argument arg, ltac::Register reg){
        //TODO
    }
    
    void move(mtac::Argument arg, ltac::FloatRegister reg){
        //TODO
    }
    
    void spills(ltac::Register reg){
        //TODO
    }

    void spills(ltac::FloatRegister reg){
        //TODO
    }
    
    void spills_if_necessary(ltac::Register reg, mtac::Argument arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
            if(!registers.inRegister(*ptr, reg)){
                spills(reg);
            }
        } else {
            spills(reg);
        }
    }
    
    void end_basic_block(){
        //TODO
    }

    /* Conversions */

    ltac::Argument to_arg(mtac::Argument arg){
        //TODO
    }

    ltac::Address to_address(std::shared_ptr<Variable> var, int offset){
        //TODO
    }
    
    ltac::Address to_address(std::shared_ptr<Variable> var, mtac::Argument offset){
        //TODO
    }
    
    /* Utility  */

    bool is_float_operator(mtac::BinaryOperator op){
        return op >= mtac::BinaryOperator::FE && op <= mtac::BinaryOperator::FL;
    }

    bool is_float_var(std::shared_ptr<Variable> variable){
        return variable->type() == BaseType::FLOAT;
    }
    
    bool is_int_var(std::shared_ptr<Variable> variable){
        return variable->type() == BaseType::INT;
    }

    template<typename Variant>
    std::shared_ptr<Variable> get_variable(Variant& variant){
        return boost::get<std::shared_ptr<Variable>>(variant);
    }

    /* Others  */
    
    template<typename T>
    void compare_binary(T& if_){
        //The first argument is not important, it can be immediate, but the second must be a register
        if(auto* ptr = boost::get<int>(&if_->arg1)){
            auto reg1 = get_free_reg();
            
            add_instruction(function, ltac::Operator::MOV, reg1, *ptr);

            auto reg2 = get_reg(get_variable(*if_->arg2));

            //The basic block must be ended before the jump
            end_basic_block();

            add_instruction(function, ltac::Operator::CMP_INT, reg1, reg2);

            registers.release(reg1);
        } else {
            auto reg1 = get_reg(get_variable(if_->arg1));
            auto reg2 = get_reg(get_variable(*if_->arg2));

            //The basic block must be ended before the jump
            end_basic_block();

            add_instruction(function, ltac::Operator::CMP_INT, reg1, reg2);
        }
    }
    
    template<typename T>
    void compare_float_binary(T& if_){
        //Comparisons of constant should have been handled by the optimizer
        assert(!(isFloat(if_->arg1) && isFloat(*if_->arg2))); 

        //If both args are variables
        if(isVariable(if_->arg1) && isVariable(*if_->arg2)){
            //The basic block must be ended before the jump
            end_basic_block();
            
            auto reg1 = get_float_reg(get_variable(if_->arg1));
            auto reg2 = get_float_reg(get_variable(*if_->arg2));

            add_instruction(function, ltac::Operator::CMP_FLOAT, reg1, reg2);
        } else if(isVariable(if_->arg1) && isFloat(*if_->arg2)){
            auto reg1 = get_float_reg(get_variable(if_->arg1));
            auto reg2 = get_free_float_reg();

            copy(*if_->arg2, reg2);

            //The basic block must be ended before the jump
            end_basic_block();

            add_instruction(function, ltac::Operator::CMP_FLOAT, reg1, reg2);
            
            float_registers.release(reg1);
        } else if(isFloat(if_->arg1) && isVariable(*if_->arg2)){
            auto reg1 = get_free_float_reg();
            auto reg2 = get_float_reg(get_variable(*if_->arg2));

            copy(if_->arg1, reg1);

            //The basic block must be ended before the jump
            end_basic_block();

            add_instruction(function, ltac::Operator::CMP_FLOAT, reg1, reg2);
            
            float_registers.release(reg1);
        }
    }

    template<typename T>
    void compare_unary(T& if_){
        if(auto* ptr = boost::get<int>(&if_->arg1)){
            auto reg = get_free_reg();
            
            add_instruction(function, ltac::Operator::MOV, reg, *ptr);

            //The basic block must be ended before the jump
            end_basic_block();
            
            add_instruction(function, ltac::Operator::OR, reg, reg);

            registers.release(reg);
        } else {
            //The basic block must be ended before the jump
            end_basic_block();
            
            auto reg = get_reg(get_variable(if_->arg1));

            add_instruction(function, ltac::Operator::OR, reg, reg);
        }
    }

    /* Visitor members  */
    
    void operator()(std::shared_ptr<mtac::IfFalse>& if_false){
        current = if_false;

        if(if_false->op){
            //Depending on the type of the operator, do a float or a int comparison
            if(is_float_operator(*if_false->op)){
                compare_float_binary(if_false);
                
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
                compare_binary(if_false);
            
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
            compare_unary(if_false);

            function->add(std::make_shared<ltac::Jump>(if_false->block->label, ltac::JumpType::Z));
        }
    }
    
    void operator()(std::shared_ptr<mtac::If>& if_){
        current = if_;

        if(if_->op){
            //Depending on the type of the operator, do a float or a int comparison
            if(is_float_operator(*if_->op)){
                compare_float_binary(if_);
            
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
                compare_binary(if_);
            
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
            compare_unary(if_);

            function->add(std::make_shared<ltac::Jump>(if_->block->label, ltac::JumpType::NZ));
        }
    }
    
    void operator()(std::shared_ptr<mtac::Goto>& goto_){
        current = goto_;

        //The basic block must be ended before the jump
        end_basic_block();

        function->add(std::make_shared<ltac::Jump>(goto_->block->label, ltac::JumpType::ALWAYS));
    }
    
    void pass_in_int_register(mtac::Argument& argument, int position){
        add_instruction(function, ltac::Operator::MOV, ltac::Register(ltac::FirstIntParam + position), to_arg(argument));
    }
    
    void pass_in_float_register(mtac::Argument& argument, int position){
        if(auto* ptr = boost::get<double>(&argument)){
            auto gpreg = get_free_reg();

            add_instruction(function, ltac::Operator::MOV, gpreg, *ptr);
            add_instruction(function, ltac::Operator::MOV, ltac::Register(ltac::FirstIntParam + position), gpreg);

            registers.release(gpreg);
        } else {
            add_instruction(function, ltac::Operator::MOV, ltac::Register(ltac::FirstIntParam + position), to_arg(argument));
        }
    }
    
    void operator()(std::shared_ptr<mtac::Param>& param){
        current = param;
        
        PlatformDescriptor* descriptor = getPlatformDescriptor(platform);
        unsigned int maxInt = descriptor->numberOfIntParamRegisters();
        unsigned int maxFloat = descriptor->numberOfFloatParamRegisters();

        if(first_param){
            if(param->function){
                for(auto& parameter : param->function->parameters){
                    auto type = param->function->getParameterType(parameter.name);
                    unsigned int position = param->function->getParameterPositionByType(parameter.name);

                    if(type == BaseType::INT && position <= maxInt){
                        ltac::Register reg(ltac::FirstIntParam + position);

                        //If the parameter register is already used by a variable or a parent parameter
                        if(registers.used(reg)){
                            if(registers[reg]->position().isParamRegister()){
                                int_pushed.push_back(reg);
                                add_instruction(function, ltac::Operator::PUSH, reg);
                            } else {
                                spills(reg);
                            }
                        }
                    }

                    if(type == BaseType::FLOAT && position <= maxFloat){
                        ltac::FloatRegister reg(ltac::FirstFloatParam + position);

                        //If the parameter register is already used by a variable or a parent parameter
                        if(float_registers.used(reg)){
                            if(float_registers[reg]->position().isParamRegister()){
                                float_pushed.push_back(reg);

                                auto gpreg = get_free_reg();
                                
                                add_instruction(function, ltac::Operator::MOV, gpreg, reg);
                                add_instruction(function, ltac::Operator::PUSH, gpreg);

                                registers.release(gpreg);
                            } else {
                                spills(reg);
                            }
                        }
                    }
                }
            }

            //The following parameters are for the same call
            first_param = false;
        }
        
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
            if(!(*ptr)->type().isArray() && is_float_var(*ptr)){
                auto reg1 = get_free_reg();
                auto reg2 = get_float_reg(*ptr);

                add_instruction(function, ltac::Operator::MOV, reg1, reg2);
                add_instruction(function, ltac::Operator::PUSH, reg1);

                registers.release(reg1);
            } else {
                if((*ptr)->type().isArray()){
                    auto position = (*ptr)->position();

                    if(position.isGlobal()){
                        auto reg = get_free_reg();

                        auto offset = size((*ptr)->type().base()) * (*ptr)->type().size();

                        add_instruction(function, ltac::Operator::MOV, reg, ltac::Address("V" + position.name()));
                        add_instruction(function, ltac::Operator::ADD, reg, offset);
                        add_instruction(function, ltac::Operator::PUSH, reg);

                        registers.release(reg);
                    } else if(position.isStack()){
                        auto reg = get_free_reg();
                        
                        add_instruction(function, ltac::Operator::MOV, reg, ltac::BP);
                        add_instruction(function, ltac::Operator::ADD, reg, -position.offset());
                        add_instruction(function, ltac::Operator::PUSH, reg);

                        registers.release(reg);
                    } else if(position.isParameter()){
                        add_instruction(function, ltac::Operator::PUSH, ltac::Address(ltac::BP, position.offset()));
                    }
                } else {
                    auto reg = get_reg(get_variable(param->arg));
                    add_instruction(function, ltac::Operator::PUSH, reg);
                }
            }
        } else if(auto* ptr = boost::get<double>(&param->arg)){
            auto reg = get_free_reg();
            add_instruction(function, ltac::Operator::MOV, reg, *ptr);
            add_instruction(function, ltac::Operator::PUSH, reg);
            registers.release(reg);
        } else {
            auto reg = get_reg(get_variable(param->arg));
            add_instruction(function, ltac::Operator::PUSH, reg);
        }
    }
    
    void operator()(std::shared_ptr<mtac::Call>& call){
        current = call;

        function->add(std::make_shared<ltac::Jump>(call->function, ltac::JumpType::CALL));

        int total = 0;
    
        PlatformDescriptor* descriptor = getPlatformDescriptor(platform);
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
        
        if(total > 0){
            add_instruction(function, ltac::Operator::FREE_STACK, total);
        }

        if(call->return_){
            if(call->return_->type() == BaseType::FLOAT){
                float_registers.setLocation(call->return_, ltac::ReturnFloat);
            } else {
                registers.setLocation(call->return_, ltac::ReturnInt1);
            }
                
            written.insert(call->return_);
        }

        if(call->return2_){
            registers.setLocation(call->return2_, ltac::ReturnInt1);
            written.insert(call->return2_);
        }

        std::reverse(int_pushed.begin(), int_pushed.end());
        std::reverse(float_pushed.begin(), float_pushed.end());

        //Restore the int parameters in registers (in the reverse order they were pushed)
        for(auto& reg : int_pushed){
            if(registers.used(reg) && registers[reg]->position().isParamRegister()){
                add_instruction(function, ltac::Operator::POP, reg);
            }
        }
        
        //Restore the float parameters in registers (in the reverse order they were pushed)
        for(auto& reg : float_pushed){
            if(float_registers.used(reg) && float_registers[reg]->position().isParamRegister()){
                add_instruction(function, ltac::Operator::POP, reg);
            }
        }

        //Each register has been restored
        int_pushed.clear();
        float_pushed.clear();

        //All the parameters have been handled by now, the next param will be the first for its call
        first_param = true;
    }

    void mul(std::shared_ptr<Variable> result, mtac::Argument& arg2){
        mtac::assertIntOrVariable(arg2);

        auto reg = get_reg(result);

        if(isInt(arg2)){
            int constant = boost::get<int>(arg2);
            
            //TODO Do these optimizations in the low level optimizer

            if(isPowerOfTwo(constant)){
                add_instruction(function, ltac::Operator::SHIFT_LEFT, reg, powerOfTwo(constant));
            } else if(constant == 3){
                add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(reg, reg, 2, 0));
            } else if(constant == 5){
                add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(reg, reg, 4, 0));
            } else if(constant == 9){
                add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(reg, reg, 8, 0));
            } else {
                add_instruction(function, ltac::Operator::MUL, reg, to_arg(arg2));
            }
        } else {
            add_instruction(function, ltac::Operator::MUL, reg, to_arg(arg2));
        }
    }
    
    //Div eax by arg2 
    void divEax(std::shared_ptr<mtac::Quadruple> quadruple){
        /*writer.stream() << "mov rdx, rax" << std::endl;
        writer.stream() << "sar rdx, 63" << std::endl;

        if(isInt(*quadruple->arg2)){
            auto reg = getReg();
            move(*quadruple->arg2, reg);

            writer.stream() << "idiv " << reg << std::endl;

            if(registers.reserved(reg)){
                registers.release(reg);
            }
        } else {
            writer.stream() << "idiv " << arg(*quadruple->arg2) << std::endl;
        }*/
    }
    
    void div(std::shared_ptr<mtac::Quadruple> quadruple){
        /*spills(Register::RDX);
        registers.reserve(Register::RDX);

        //Form x = x / y
        if(*quadruple->arg1 == quadruple->result){
            safeMove(quadruple->result, Register::RAX);

            divEax(quadruple);
            //Form x = y / z (y: variable)
        } else if(isVariable(*quadruple->arg1)){
            spills(Register::RAX);
            registers.reserve(Register::RAX);

            copy(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), Register::RAX);

            divEax(quadruple);

            registers.release(Register::RAX);
            registers.setLocation(quadruple->result, Register::RAX);
        } else {
            spills(Register::RAX);
            registers.reserve(Register::RAX);

            copy(*quadruple->arg1, Register::RAX);

            divEax(quadruple);

            registers.release(Register::RAX);
            registers.setLocation(quadruple->result, Register::RAX);
        }

        registers.release(Register::RDX);*/
    }
    
    void mod(std::shared_ptr<mtac::Quadruple> quadruple){
        /*spills(Register::RAX);
        spills(Register::RDX);

        registers.reserve(Register::RAX);
        registers.reserve(Register::RDX);

        copy(*quadruple->arg1, Register::RAX);

        divEax(quadruple);

        //result is in edx (no need to move it now)
        registers.setLocation(quadruple->result, Register::RDX);

        registers.release(Register::RAX);*/
    }
    
    void set_if_cc(ltac::Operator set, std::shared_ptr<mtac::Quadruple>& quadruple){
        /*Register reg = getRegNoMove(quadruple->result);

        //The first argument is not important, it can be immediate, but the second must be a register
        if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
            auto reg = getReg();

            writer.stream() << "mov " << reg << ", " << *ptr << std::endl;

            writer.stream() << "cmp " << reg << ", " << arg(*quadruple->arg2) << std::endl;

            registers.release(reg);
        } else {
            writer.stream() << "cmp " << arg(*quadruple->arg1) << ", " << arg(*quadruple->arg2) << std::endl;
        }

        //TODO Find a better way to achieve that
        Register valueReg = getReg();
        writer.stream() << "mov " << valueReg << ", 1" << std::endl;
        writer.stream() << set << " " << reg << ", " << valueReg << std::endl;
        registers.release(valueReg);

        written.insert(quadruple->result);*/
    }
    
    void operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
        current = quadruple;

        switch(quadruple->op){
            case mtac::Operator::ASSIGN:
            {
                //TODO This optimization can be done in the low level optimizer

                //The fastest way to set a register to 0 is to use xorl
                if(mtac::equals<int>(*quadruple->arg1, 0)){
                    auto reg = get_reg_no_move(quadruple->result);
                    add_instruction(function, ltac::Operator::XOR, reg, reg);
                } 
                //In all the others cases, just move the value to the register
                else {
                    auto reg = get_reg_no_move(quadruple->result);
                    add_instruction(function, ltac::Operator::MOV, reg, to_arg(*quadruple->arg1));
                }

                written.insert(quadruple->result);

                break;
            }
            case mtac::Operator::FASSIGN:
            {
                //The fastest way to set a register to 0 is to use pxor
                if(mtac::equals<int>(*quadruple->arg1, 0)){
                    auto reg = get_float_reg_no_move(quadruple->result);
                    add_instruction(function, ltac::Operator::XOR, reg, reg);
                } 
                //In all the others cases, just move the value to the register
                else {
                    auto reg = get_float_reg_no_move(quadruple->result);
                    copy(*quadruple->arg1, reg);
                }

                written.insert(quadruple->result);

                break;
            }
            case mtac::Operator::ADD:
            {
                auto result = quadruple->result;

                //Optimize the special form a = a + b by using only one instruction
                if(*quadruple->arg1 == result){
                    auto reg = get_reg(quadruple->result);

                    //TODO Do this optimization in the peephole optimizer

                    //a = a + 1 => increment a
                    if(*quadruple->arg2 == 1){
                        add_instruction(function, ltac::Operator::INC, reg);
                    }
                    //a = a + -1 => decrement a
                    else if(*quadruple->arg2 == -1){
                        add_instruction(function, ltac::Operator::DEC, reg);
                    }
                    //In the other cases, perform a simple addition
                    else {
                        add_instruction(function, ltac::Operator::ADD, reg, to_arg(*quadruple->arg2));
                    }
                } 
                //Optimize the special form a = b + a by using only one instruction
                else if(*quadruple->arg2 == result){
                    auto reg = get_reg(quadruple->result);

                    //a = 1 + a => increment a
                    if(*quadruple->arg1 == 1){
                        add_instruction(function, ltac::Operator::INC, reg);
                    }
                    //a = -1 + a => decrement a
                    else if(*quadruple->arg1 == -1){
                        add_instruction(function, ltac::Operator::DEC, reg);
                    }
                    //In the other cases, perform a simple addition
                    else {
                        add_instruction(function, ltac::Operator::ADD, reg, to_arg(*quadruple->arg2));
                    }
                } 
                //In the other cases, use lea to perform the addition
                else {
                    auto reg = get_reg_no_move(quadruple->result);
                    add_instruction(function, ltac::Operator::LEA, reg, ltac::Address(get_reg(get_variable(*quadruple->arg1)), get_reg(get_variable(*quadruple->arg2))));
                }
        
                written.insert(quadruple->result);

                break;
            }
            case mtac::Operator::SUB:
            {
                auto result = quadruple->result;

                //Optimize the special form a = a - b by using only one instruction
                if(*quadruple->arg1 == result){
                    auto reg = get_reg(quadruple->result);

                    //a = a - 1 => decrement a
                    if(*quadruple->arg2 == 1){
                        add_instruction(function, ltac::Operator::DEC, reg);
                    }
                    //a = a - -1 => increment a
                    else if(*quadruple->arg2 == -1){
                        add_instruction(function, ltac::Operator::INC, reg);
                    }
                    //In the other cases, perform a simple subtraction
                    else {
                        add_instruction(function, ltac::Operator::SUB, reg, to_arg(*quadruple->arg2));
                    }
                } 
                //In the other cases, move the first arg into the result register and then subtract the second arg into it
                else {
                    auto reg = get_reg_no_move(quadruple->result);
                    add_instruction(function, ltac::Operator::MOV, reg, to_arg(*quadruple->arg1));
                    add_instruction(function, ltac::Operator::SUB, reg, to_arg(*quadruple->arg2));
                }
                
                written.insert(quadruple->result);
                
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
                    add_instruction(function, ltac::Operator::MUL, get_reg_no_move(quadruple->result), to_arg(*quadruple->arg1), to_arg(*quadruple->arg2));
                }
                //Form x = y * z (y: immediate)
                else if(isInt(*quadruple->arg1) && isVariable(*quadruple->arg2)){
                    add_instruction(function, ltac::Operator::MUL, get_reg_no_move(quadruple->result), to_arg(*quadruple->arg2), to_arg(*quadruple->arg1));
                }
                //Form x = y * z (both variables)
                else if(isVariable(*quadruple->arg1) && isVariable(*quadruple->arg2)){
                    auto reg = get_reg_no_move(quadruple->result);
                    copy(*quadruple->arg1, reg);
                    add_instruction(function, ltac::Operator::MUL, reg, to_arg(*quadruple->arg2));
                }
                
                written.insert(quadruple->result);

                break;            
            }
            case mtac::Operator::DIV:
                //TODO Optimization in the peephole optimizer
                //Form x = x / y when y is power of two
                if(*quadruple->arg1 == quadruple->result && isInt(*quadruple->arg2)){
                    int constant = boost::get<int>(*quadruple->arg2);

                    if(isPowerOfTwo(constant)){
                        add_instruction(function, ltac::Operator::SHIFT_RIGHT, get_reg(quadruple->result), powerOfTwo(constant));

                        written.insert(quadruple->result);

                        return;
                    }
                }

                div(quadruple);

                written.insert(quadruple->result);
                
                break;            
            case mtac::Operator::MOD:
                mod(quadruple);
                
                written.insert(quadruple->result);

                break;
            case mtac::Operator::FADD:
            {
                auto result = quadruple->result;
                    
                //Optimize the special form a = a + b
                if(*quadruple->arg1 == result){
                    auto reg = get_float_reg(result);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FADD, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg2));
                    }
                }
                //Optimize the special form a = b + a by using only one instruction
                else if(*quadruple->arg2 == result){
                    auto reg = get_float_reg(result);

                    if(mtac::isFloat(*quadruple->arg1)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg1, reg2);
                        add_instruction(function, ltac::Operator::FADD, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg1));
                    }
                }
                //In the other forms, use two instructions
                else {
                    auto reg = get_float_reg_no_move(result);
                    copy(*quadruple->arg1, reg);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FADD, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FADD, reg, to_arg(*quadruple->arg2));
                    }
                }
        
                written.insert(quadruple->result);

                break;
            }
            case mtac::Operator::FSUB:
            {
                auto result = quadruple->result;

                //Optimize the special form a = a - b
                if(*quadruple->arg1 == result){
                    auto reg = get_float_reg(result);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FSUB, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FSUB, reg, to_arg(*quadruple->arg2));
                    }
                } else {
                    auto reg = get_float_reg_no_move(result);
                    copy(*quadruple->arg1, reg);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FSUB, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FSUB, reg, to_arg(*quadruple->arg2));
                    }
                }
                
                written.insert(quadruple->result);
                
                break;
            }
            case mtac::Operator::FMUL:
                //Form  x = x * y
                if(*quadruple->arg1 == quadruple->result){
                    auto reg = get_float_reg(quadruple->result);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FMUL, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
                    }
                }
                //Form x = y * x
                else if(*quadruple->arg2 == quadruple->result){
                    auto reg = get_float_reg(quadruple->result);

                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FMUL, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
                    }
                } 
                //General form
                else  {
                    auto reg = get_float_reg_no_move(quadruple->result);
                    copy(*quadruple->arg1, reg);
                    
                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FMUL, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FMUL, reg, to_arg(*quadruple->arg2));
                    }
                }
                
                written.insert(quadruple->result);

                break;            
            case mtac::Operator::FDIV:
                //Form x = x / y
                if(*quadruple->arg1 == quadruple->result){
                    auto reg = get_float_reg(quadruple->result);
                    
                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FDIV, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FDIV, reg, to_arg(*quadruple->arg2));
                    }
                } 
                //General form
                else {
                    auto reg = get_float_reg_no_move(quadruple->result);
                    copy(*quadruple->arg1, reg);
                    
                    if(mtac::isFloat(*quadruple->arg2)){
                        auto reg2 = get_free_float_reg();
                        copy(*quadruple->arg2, reg2);
                        add_instruction(function, ltac::Operator::FDIV, reg, reg2);
                        float_registers.release(reg2);
                    } else {
                        add_instruction(function, ltac::Operator::FDIV, reg, to_arg(*quadruple->arg2));
                    }
                }
                
                written.insert(quadruple->result);
                
                break;            
            case mtac::Operator::I2F:
            {
                //Constants should have been replaced by the optimizer
                assert(isVariable(*quadruple->arg1));

                auto reg = get_reg(get_variable(*quadruple->arg1));
                auto resultReg = get_float_reg_no_move(quadruple->result);

                add_instruction(function, ltac::Operator::I2F, resultReg, reg);
        
                written.insert(quadruple->result);

                break;
            }
            case mtac::Operator::F2I:
            {
                //Constants should have been replaced by the optimizer
                assert(isVariable(*quadruple->arg1));

                auto reg = get_float_reg(get_variable(*quadruple->arg1));
                auto resultReg = get_reg_no_move(quadruple->result);

                add_instruction(function, ltac::Operator::F2I, resultReg, reg);
        
                written.insert(quadruple->result);

                break;
            }
            case mtac::Operator::MINUS:
            {
                //Constants should have been replaced by the optimizer
                assert(isVariable(*quadruple->arg1));

                add_instruction(function, ltac::Operator::NEG, get_reg(get_variable(*quadruple->arg1)));
                
                written.insert(quadruple->result);

                break;
            }
            case mtac::Operator::FMINUS:
            {
                //Constants should have been replaced by the optimizer
                assert(isVariable(*quadruple->arg1));

                auto reg = get_free_float_reg();
                copy(-1.0, reg);

                add_instruction(function, ltac::Operator::FMUL, get_float_reg(get_variable(*quadruple->arg1)), reg);

                float_registers.release(reg);

                written.insert(quadruple->result);
                
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

               auto reg = get_reg_no_move(quadruple->result);
               add_instruction(function, ltac::Operator::MOV, reg, to_address(variable, offset));
    
               written.insert(quadruple->result);

               break;
            }
            case mtac::Operator::FDOT:
            {
               assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));
               assert(boost::get<int>(&*quadruple->arg2));

               auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
               int offset = boost::get<int>(*quadruple->arg2);

               auto reg = get_float_reg_no_move(quadruple->result);
               add_instruction(function, ltac::Operator::MOV, reg, to_address(variable, offset));
    
               written.insert(quadruple->result);

               break;
            }
            case mtac::Operator::DOT_ASSIGN:
            {
                ASSERT(boost::get<int>(&*quadruple->arg1), "The offset must be be an int");

                int offset = boost::get<int>(*quadruple->arg1);

                add_instruction(function, ltac::Operator::MOV, to_address(quadruple->result, offset), to_arg(*quadruple->arg2));

                break;
            }
            case mtac::Operator::DOT_FASSIGN:
            {
                ASSERT(boost::get<int>(&*quadruple->arg1), "The offset must be be an int");

                int offset = boost::get<int>(*quadruple->arg1);
                auto reg = get_free_float_reg();
                copy(*quadruple->arg2, reg);

                add_instruction(function, ltac::Operator::FMOV, to_address(quadruple->result, offset), reg);

                break;
            }
            case mtac::Operator::ARRAY:
            {
                assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));

                if(is_float_var(quadruple->result)){
                    auto reg = get_float_reg_no_move(quadruple->result);

                    add_instruction(function, ltac::Operator::FMOV, reg, to_address(get_variable(*quadruple->arg1), *quadruple->arg2));
                } else {
                    auto reg = get_reg_no_move(quadruple->result);

                    add_instruction(function, ltac::Operator::MOV, reg, to_address(get_variable(*quadruple->arg1), *quadruple->arg2));
                }
               
                written.insert(quadruple->result);
                
                break;            
            }
            case mtac::Operator::ARRAY_ASSIGN:
                if(quadruple->result->type().base() == BaseType::FLOAT){
                    auto reg = get_free_float_reg();
                    copy(*quadruple->arg2, reg);

                    add_instruction(function, ltac::Operator::FMOV, to_address(quadruple->result, *quadruple->arg1), reg);

                    float_registers.release(reg);
                } else {
                    add_instruction(function, ltac::Operator::MOV, to_address(quadruple->result, *quadruple->arg1), to_arg(*quadruple->arg2));
                }
                
                break;
            case mtac::Operator::RETURN:
            {
                //A return without args is the same as exiting from the function
                if(quadruple->arg1){
                    if(isFloat(*quadruple->arg1)){
                        spills(ltac::ReturnFloat);
                        move(*quadruple->arg1, ltac::ReturnFloat);
                    } else if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1) && is_float_var(get_variable(*quadruple->arg1))){
                        auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

                        auto reg = get_float_reg(variable);
                        if(reg != ltac::ReturnFloat){
                            spills(ltac::ReturnFloat);
                            add_instruction(function, ltac::Operator::FMOV, ltac::ReturnFloat, reg);
                        }
                    } else {
                        auto reg1 = ltac::ReturnInt1;
                        auto reg2 = ltac::ReturnInt2;

                        spills_if_necessary(reg1, *quadruple->arg1);

                        bool necessary = true;
                        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                            if(registers.inRegister(*ptr, reg1)){
                                necessary = false;
                            }
                        }    

                        if(necessary){
                            add_instruction(function, ltac::Operator::MOV, reg1, to_arg(*quadruple->arg1));
                        }

                        if(quadruple->arg2){
                            spills_if_necessary(reg2, *quadruple->arg2);

                            necessary = true;
                            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                                if(registers.inRegister(*ptr, reg2)){
                                    necessary = false;
                                }
                            }    

                            if(necessary){
                                add_instruction(function, ltac::Operator::MOV, reg2, to_arg(*quadruple->arg2));
                            }
                        }
                    }
                }
    
                if(function->context->size() > 0){
                    add_instruction(function, ltac::Operator::FREE_STACK, function->context->size());
                }

                //The basic block must be ended before the jump
                end_basic_block();

                add_instruction(function, ltac::Operator::LEAVE);

                break;
            }
        }
    }

    void operator()(mtac::NoOp&){
        //Nothing to do
    }
    
    void operator()(std::string& str){
        function->add(str);
    }
};

} //end of anonymous namespace

void ltac::Compiler::compile(std::shared_ptr<mtac::BasicBlock> block, std::shared_ptr<ltac::Function> target_function){
    //Handle parameters
    
    //If necessary add a label for the block
    if(block_usage.find(block) != block_usage.end()){
        target_function->add(block->label);
    }

    //TODO Fill the registers
    StatementCompiler compiler({}, {}, target_function);
    
    for(unsigned int i = 0; i < block->statements.size(); ++i){
        auto& statement = block->statements[i];

        if(i == block->statements.size() - 1){
            compiler.last = true;
        } else {
            compiler.next = block->statements[i+1];
        }
        
        visit(compiler, statement);
    }

    //end basic block
}

