//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "FunctionContext.hpp"
#include "Labels.hpp"
#include "VisitorUtils.hpp"
#include "Platform.hpp"

#include "asm/Registers.hpp"

#include "ltac/Compiler.hpp"

#include "mtac/Utils.hpp" //TODO Perhaps part of this should be moved to ltac ? 

using namespace eddic;

void add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1){
    function->add(std::make_shared<ltac::Instruction>(op, arg1));
}

void add_instruction(std::shared_ptr<ltac::Function> function, ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2){
    function->add(std::make_shared<ltac::Instruction>(op, arg1, arg2));
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
    
    ltac::FloatRegister get_free_float_reg(){
        //TODO
    }

    ltac::FloatRegister get_float_reg(std::shared_ptr<Variable> var){
        //TODO
    }

    void copy(mtac::Argument& arg, ltac::Register reg){
        //TODO
    }
    
    void copy(mtac::Argument& arg, ltac::FloatRegister reg){
        //TODO
    }

    void end_basic_block(){
        //TODO
    }
    
    /* Utility  */

    bool is_float_operator(mtac::BinaryOperator op){
        return op >= mtac::BinaryOperator::FE && op <= mtac::BinaryOperator::FL;
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
    
    void operator()(std::shared_ptr<mtac::Param>& param){
        current = param;

        //TODO
    }
    
    void operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
        current = quadruple;

        //TODO
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

