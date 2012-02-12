//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

#include <boost/variant.hpp>

#include "FunctionContext.hpp"
#include "AssemblyFileWriter.hpp"
#include "Types.hpp"
#include "Variable.hpp"
#include "Utils.hpp"
#include "Labels.hpp"
#include "VisitorUtils.hpp"
#include "FunctionTable.hpp"

#include "asm/IntelX86CodeGenerator.hpp"
#include "asm/Registers.hpp"

#include "tac/Utils.hpp"

using namespace eddic;

using eddic::tac::isVariable;
using eddic::tac::isInt;

as::IntelX86CodeGenerator::IntelX86CodeGenerator(AssemblyFileWriter& w) : IntelCodeGenerator(w) {}

namespace eddic { namespace as {

enum Register {
    EAX,
    EBX,
    ECX,
    EDX,

    ESP, //Extended stack pointer
    EBP, //Extended base pointer

    ESI, //Extended source index
    EDI, //Extended destination index
    
    REGISTER_COUNT  
};

std::string regToString(Register reg){
    static std::string registers[Register::REGISTER_COUNT] = {"eax", "ebx", "ecx", "edx", "esp", "ebp", "esi", "edi"};

    return registers[reg];
}

std::ostream& operator<<(std::ostream& os, Register reg){
    os << regToString(reg);

    return os;
}

std::string operator+(const char* left, Register right) {
    return left + regToString(right);
}

std::string operator+(std::string left, Register right) {
    return left + regToString(right);
}

void enterFunction(AssemblyFileWriter& writer){
    writer.stream() << "push ebp" << std::endl;
    writer.stream() << "mov ebp, esp" << std::endl;
}

void defineFunction(AssemblyFileWriter& writer, const std::string& function){
    writer.stream() << std::endl << function << ":" << std::endl;
    
    enterFunction(writer);
}

void leaveFunction(AssemblyFileWriter& writer){
    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

struct StatementCompiler : public boost::static_visitor<> {
    AssemblyFileWriter& writer;
    std::shared_ptr<tac::Function> function;

    std::unordered_map<std::shared_ptr<tac::BasicBlock>, std::string> labels;
    std::unordered_set<std::shared_ptr<tac::BasicBlock>> blockUsage;

    Registers<Register> registers;
    std::unordered_set<std::shared_ptr<Variable>> written;

    tac::Statement current;
    tac::Statement next;

    bool last;
    bool ended;

    StatementCompiler(AssemblyFileWriter& w, std::shared_ptr<tac::Function> f) : writer(w), function(f), 
            registers({EDI, ESI, ECX, EDX, EBX, EAX}, std::make_shared<Variable>("__fake__", Type(BaseType::INT, false), Position(PositionType::TEMPORARY))){
        last = ended = false;
    }

    //Called at the beginning of each basic block
    void reset(){
        registers.reset();
        written.clear();

        last = ended = false;
    }

    void setNext(tac::Statement statement){
        next = statement;
    }

    bool isLive(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, std::shared_ptr<Variable> variable){
        if(liveness.find(variable) != liveness.end()){
            return liveness[variable];   
        } else {
            return !variable->position().isTemporary();
        }
    }

    bool isLive(std::shared_ptr<Variable> variable, tac::Statement statement){
        if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&statement)){
            return isLive((*ptr)->liveness, variable);
        } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
            return isLive((*ptr)->liveness, variable);
        } 

        assert(false); //No liveness calculations in the other cases
    }

    bool isNextLive(std::shared_ptr<Variable> variable){
        if(last){
            return !variable->position().isTemporary();
        } else {             
            return isLive(variable, next);
        }
    }   

    bool isLive(std::shared_ptr<Variable> variable){
        return isLive(variable, current);
    }

    void copy(tac::Argument argument, Register reg){
        if(auto* ptr = boost::get<int>(&argument)){
            writer.stream() << "mov " << reg << ", " << ::toString(*ptr) << std::endl;
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            writer.stream() << "mov " << reg << ", " << *ptr << std::endl;
        } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            auto variable = *ptr;

            //If the variable is hold in a register, just move the register value
            if(registers.inRegister(variable)){
                auto oldReg = registers[variable];
                
                writer.stream() << "mov " << reg << ", " << oldReg << std::endl;
            } else {
                auto position = variable->position();

                if(position.isStack()){
                    writer.stream() << "mov " << reg << ", [ebp + " << (-1 * position.offset()) << "]" << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << "mov " << reg << ", [ebp + " << position.offset() << "]" << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << "mov " << reg << ", [V" << position.name() << "]" << std::endl;
                } else if(position.isTemporary()){
                    //The temporary should have been handled by the preceding condition (hold in a register)
                    assert(false);
                }
            } 
        }
    }

    void move(tac::Argument argument, Register reg){
        if(auto* ptr = boost::get<int>(&argument)){
            writer.stream() << "mov " << reg << ", " << ::toString(*ptr) << std::endl;
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            writer.stream() << "mov " << reg << ", " << *ptr << std::endl;
        } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            auto variable = *ptr;

            //If the variable is hold in a register, just move the register value
            if(registers.inRegister(variable)){
                auto oldReg = registers[variable];
               
                //Only if the variable is not already on the same register 
                if(oldReg != reg){
                    writer.stream() << "mov " << reg << ", " << oldReg << std::endl;

                    //There is nothing more in the old register
                    registers.remove(variable);
                }
            } else {
                auto position = variable->position();

                if(position.isStack()){
                    writer.stream() << "mov " << reg << ", [ebp + " << (-1 * position.offset()) << "]" << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << "mov " << reg << ", [ebp + " << position.offset() << "]" << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << "mov " << reg << ", [V" << position.name() << "]" << std::endl;
                } else if(position.isTemporary()){
                    //The temporary should have been handled by the preceding condition (hold in a register)
                    assert(false);
                }
            } 
            
            //The variable is now held in the new register
            registers.setLocation(variable, reg);
        }
    }

    void safeMove(std::shared_ptr<Variable> variable, Register reg){
        if(registers.used(reg)){
            if(registers[reg] != variable){
                spills(reg);

                move(variable, reg);
            }
        } else {
            move(variable, reg);
        }
    }

    void spills(Register reg){
        //If the register is not used, there is nothing to spills
        if(registers.used(reg)){
            auto variable = registers[reg];

            //If the variable has not been written, there is no need to spill it
            if(written.find(variable) != written.end()){
                auto position = variable->position();
                if(position.isStack()){
                    writer.stream() << "mov [ebp + " << (-1 * position.offset()) << "], " << reg << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << "mov [ebp + " << position.offset() << "], " << reg << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << "mov [V" << position.name() << "], " << reg << std::endl;
                } else if(position.isTemporary()){
                    //If the variable is live, move it to another register, else do nothing
                    if(isLive(variable)){
                        registers.remove(variable);
                        registers.reserve(reg);

                        Register newReg = getReg(variable, false);
                        writer.stream() << "mov " << newReg << ", " << reg << std::endl;

                        registers.release(reg);

                        return; //Return here to avoid erasing variable from variables
                    }
                }
            }
            
            //The variable is no more contained in the register
            registers.remove(variable);

            //The variable has not been written now
            written.erase(variable);
        }
    }

    Register getFreeReg(){
        //Try to get a free register 
        for(auto reg : registers){
            if(!registers.used(reg)){
                return reg;
            } else if(!registers.reserved(reg) && !isLive(registers[reg])){
                registers.remove(registers[reg]);

                return reg;
            }
        }
       
        //There are no free register, take one
        auto reg = registers.first();
        bool found = false;

        //First, try to take a register that doesn't need to be spilled (variable has not modified)
        for(auto remaining : registers){
            if(!registers.reserved(remaining)){
                if(written.find(registers[remaining]) == written.end()){
                    reg = remaining;
                    found = true;
                }
            }
        }
       
        //If there is no registers that doesn't need to be spilled, take the first one not reserved 
        if(!found){
            for(auto remaining : registers){
                if(!registers.reserved(remaining)){
                    reg = remaining;
                    found = true;
                }
            }
        }

        assert(found);
        spills(reg);
        
        return reg; 
    }
    
    Register getReg(std::shared_ptr<Variable> variable, bool doMove){
        //The variable is already in a register
        if(registers.inRegister(variable)){
            return registers[variable];
        }
       
        Register reg = getFreeReg();

        if(doMove){
            move(variable, reg);
        }

        registers.setLocation(variable, reg);

        return reg;
    }
    
    Register getRegNoMove(std::shared_ptr<Variable> variable){
        return getReg(variable, false);
    }

    Register getReg(std::shared_ptr<Variable> variable){
        return getReg(variable, true);
    }
    
    Register getReg(){
        Register reg = getFreeReg();

        registers.reserve(reg);

        return reg;
    }
    
    std::string toString(std::shared_ptr<Variable> variable, int offset){
        auto position = variable->position();

        if(position.isStack()){
            return "[ebp + " + ::toString(-position.offset() + offset) + "]";
        } else if(position.isParameter()){
            //The case of array is special because only the address is passed, not the complete array
            if(variable->type().isArray())
            {
                //TODO This register allocation is not safe
                Register reg = getReg();

                writer.stream() << "mov " << reg << ", [ebp + " << ::toString(position.offset()) << "]" << std::endl;

                registers.release(reg);

                return "[" + reg + "+" + ::toString(offset) + "]";
            } 
            //In the other cases, the value is passed, so we can compute the offset directly
            else {
                return "[ebp + " + ::toString(position.offset() + offset) + "]";
            }
        } else if(position.isGlobal()){
            return "[V" + position.name() + "+" + ::toString(offset) + "]";
        } else if(position.isTemporary()){
            assert(false); //We are in da shit
        }

        assert(false);
    }
    
    std::string toString(std::shared_ptr<Variable> variable, tac::Argument offset){
        if(auto* ptr = boost::get<int>(&offset)){
            return toString(variable, *ptr);
        }
        
        assert(boost::get<std::shared_ptr<Variable>>(&offset));

        auto* offsetVariable = boost::get<std::shared_ptr<Variable>>(&offset);
        auto position = variable->position();

        auto offsetReg = getReg(*offsetVariable);
        
        if(position.isStack()){
            return "[ebp + " + ::toString(-1 * (position.offset())) + "]";//TODO Verify
        } else if(position.isParameter()){
            //TODO This register allocation is not safe
            Register reg = getReg();
            
            writer.stream() << "mov " << reg << ", [ebp + " << ::toString(position.offset()) << "]" << std::endl;

            registers.release(reg);

            return "[" + reg + "+" + offsetReg + "]";
        } else if(position.isGlobal()){
            return "[" + offsetReg + "+V" + position.name() + "]";
        } else if(position.isTemporary()){
            assert(false); //We are in da shit
        }

        assert(false);
    }

    std::string arg(tac::Argument argument){
        if(auto* ptr = boost::get<int>(&argument)){
            return ::toString(*ptr);
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            return *ptr;//TODO Verify that
        } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            if((*ptr)->position().isTemporary()){
                return regToString(getReg(*ptr, false));
            } else {
                return regToString(getReg(*ptr, true));
            }
        }

        assert(false);
    }

    void operator()(std::shared_ptr<tac::Goto>& goto_){
        current = goto_;

        //The basic block must be ended before the jump
        endBasicBlock();

        writer.stream() << "jmp " << labels[goto_->block] << std::endl; 
    }

    void operator()(std::shared_ptr<tac::Call>& call){
        current = call;

        writer.stream() << "call " << call->function << std::endl;
        
        if(call->params > 0){
            writer.stream() << "add esp, " << call->params << std::endl;
        }

        if(call->return_){
            registers.setLocation(call->return_, Register::EAX);
            written.insert(call->return_);
        }

        if(call->return2_){
            registers.setLocation(call->return2_, Register::EBX);
            written.insert(call->return2_);
        }
    }
   
    void spillsIfNecessary(Register reg, tac::Argument arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
            if(!registers.inRegister(*ptr, reg)){
                spills(reg);
            }
        } else {
            spills(reg);
        }
    }
   
    void setLast(bool l){
        last = l;
    }
    
    void endBasicBlock(){
        //End the basic block
        for(auto reg : registers){
            //The register can be reserved if the ending occurs in a special break case
            if(!registers.reserved(reg) && registers.used(reg)){
                auto variable = registers[reg];

                if(!variable->position().isTemporary()){
                    spills(reg);    
                }
            }
        }

        ended = true;
    }

    std::string toSubRegister(Register reg){
        switch(reg){
            case Register::EAX:
                return "ah";
            case Register::EBX:
                return "bh";
            case Register::ECX:
                return "ch";
            case Register::EDX:
                return "dh";
            case Register::EDI:
                return "di";
            case Register::ESI:
                return "si";
            default:
                assert(false);
        }
    }

    void setIfCc(const std::string& set, std::shared_ptr<tac::Quadruple>& quadruple){
        //We use EAX in order to avoid esi and edi that have not 8 byte version
        spills(Register::EAX);

        Register reg = Register::EAX;

        registers.setLocation(quadruple->result, reg);
        
        writer.stream() << "xor " << reg << ", " << reg << std::endl;

        //The first argument is not important, it can be immediate, but the second must be a register
        if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
            auto reg = getReg();

            writer.stream() << "mov " << reg << ", " << *ptr << std::endl;

            writer.stream() << "cmp " << reg << ", " << arg(*quadruple->arg2) << std::endl;

            registers.release(reg);
        } else {
            writer.stream() << "cmp " << arg(*quadruple->arg1) << ", " << arg(*quadruple->arg2) << std::endl;
        }

        writer.stream() << set << " " << toSubRegister(reg) << std::endl;
                    
        written.insert(quadruple->result);
    }

    void mul(std::shared_ptr<Variable> result, tac::Argument arg2){
        tac::assertIntOrVariable(arg2);

        if(isInt(arg2)){
            int constant = boost::get<int>(arg2);

            if(isPowerOfTwo(constant)){
                writer.stream() << "sal " << arg(result) << ", " << powerOfTwo(constant) << std::endl;
            } else if(constant == 3){
                writer.stream() << "lea " << arg(result) << ", [" << arg(result) << " * 2 + " << arg(result) << "]" << std::endl;
            } else if(constant == 5){
                writer.stream() << "lea " << arg(result) << ", [" << arg(result) << " * 4 + " << arg(result) << "]" << std::endl;
            } else if(constant == 9){
                writer.stream() << "lea " << arg(result) << ", [" << arg(result) << " * 8 + " << arg(result) << "]" << std::endl;
            } else {
                writer.stream() << "imul " << arg(result) << ", " << arg(arg2) << std::endl; 
            }
        } else {
            writer.stream() << "imul " << arg(result) << ", " << arg(arg2) << std::endl; 
        }
    }
  
    //Div eax by arg2 
    void div(std::shared_ptr<tac::Quadruple> quadruple){
        writer.stream() << "mov edx, eax" << std::endl;
        writer.stream() << "sar edx, 31" << std::endl;

        if(isInt(*quadruple->arg2)){
            auto reg = getReg();
            move(*quadruple->arg2, reg);

            writer.stream() << "idiv " << reg << std::endl;

            if(registers.reserved(reg)){
                registers.release(reg);
            }
        } else {
            writer.stream() << "idiv " << arg(*quadruple->arg2) << std::endl;
        }
    }
    
    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        current = quadruple;
        
        if(!quadruple->op){
            //The fastest way to set a register to 0 is to use xorl
            if(tac::equals<int>(*quadruple->arg1, 0)){
                Register reg = getRegNoMove(quadruple->result);
                writer.stream() << "xor " << reg << ", " << reg << std::endl;            
            } 
            //In all the others cases, just move the value to the register
            else {
                Register reg = getRegNoMove(quadruple->result);
                writer.stream() << "mov " << reg << ", " << arg(*quadruple->arg1) << std::endl;            
            }

            written.insert(quadruple->result);
        } else {
            switch(*quadruple->op){
                case tac::Operator::ADD:
                {
                    auto result = quadruple->result;

                    //Optimize the special form a = a + b by using only one instruction
                    if(*quadruple->arg1 == result){
                        Register reg = getReg(quadruple->result);
                        
                        //a = a + 1 => increment a
                        if(*quadruple->arg2 == 1){
                            writer.stream() << "inc " << reg << std::endl;
                        }
                        //a = a + -1 => decrement a
                        else if(*quadruple->arg2 == -1){
                            writer.stream() << "dec " << reg << std::endl;
                        }
                        //In the other cases, perform a simple addition
                        else {
                            writer.stream() << "add " << reg << ", " << arg(*quadruple->arg2) << std::endl;
                        }
                    } 
                    //Optimize the special form a = b + a by using only one instruction
                    else if(*quadruple->arg2 == result){
                        Register reg = getReg(quadruple->result);
                        
                        //a = 1 + a => increment a
                        if(*quadruple->arg1 == 1){
                            writer.stream() << "inc " << reg << std::endl;
                        }
                        //a = -1 + a => decrement a
                        else if(*quadruple->arg1 == -1){
                            writer.stream() << "dec " << reg << std::endl;
                        }
                        //In the other cases, perform a simple addition
                        else {
                            writer.stream() << "add " << reg << ", " << arg(*quadruple->arg1) << std::endl;
                        }
                    } 
                    //In the other cases, use lea to perform the addition
                    else {
                        Register reg = getRegNoMove(quadruple->result);
                        writer.stream() << "lea " << reg << ", [" << arg(*quadruple->arg1) << " + " << arg(*quadruple->arg2) << "]" << std::endl;
                    }
            
                    written.insert(quadruple->result);

                    break;
                }
                case tac::Operator::SUB:
                {
                    auto result = quadruple->result;
                    
                    //Optimize the special form a = a - b by using only one instruction
                    if(*quadruple->arg1 == result){
                        Register reg = getReg(quadruple->result);
                        
                        //a = a - 1 => decrement a
                        if(*quadruple->arg2 == 1){
                            writer.stream() << "dec " << reg << std::endl;
                        }
                        //a = a - -1 => increment a
                        else if(*quadruple->arg2 == -1){
                            writer.stream() << "inc " << reg << std::endl;
                        }
                        //In the other cases, perform a simple subtraction
                        else {
                            writer.stream() << "sub " << reg << ", " << arg(*quadruple->arg2) << std::endl;
                        }
                    } 
                    //In the other cases, move the first arg into the result register and then subtract the second arg into it
                    else {
                        Register reg = getRegNoMove(quadruple->result);
                        writer.stream() << "mov " << reg << ", " << arg(*quadruple->arg1) << std::endl;
                        writer.stream() << "sub " << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                    
                    written.insert(quadruple->result);
                    
                    break;
                }
                case tac::Operator::MUL:
                {
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
                        writer.stream() << "imul " << arg(quadruple->result) << ", " << arg(*quadruple->arg1) << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                    //Form x = y * z (y: immediate)
                    else if(isInt(*quadruple->arg1) && isVariable(*quadruple->arg2)){
                        writer.stream() << "imul " << arg(quadruple->result) << ", " << arg(*quadruple->arg2) << ", " << arg(*quadruple->arg1) << std::endl;
                    }
                    //Form x = y * z (both variables)
                    else if(isVariable(*quadruple->arg1) && isVariable(*quadruple->arg2)){
                        auto reg = getReg(quadruple->result, false);
                        copy(*quadruple->arg1, reg);
                        writer.stream() << "imul " << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    } else {
                        //This case should never happen unless the optimizer has bugs
                        assert(false);
                    }
                    
                    written.insert(quadruple->result);

                    break;            
                }
                case tac::Operator::DIV:
                    //Form x = x / y when y is power of two
                    if(*quadruple->arg1 == quadruple->result && isInt(*quadruple->arg2)){
                        int constant = boost::get<int>(*quadruple->arg2);

                        if(isPowerOfTwo(constant)){
                            writer.stream() << "sar " << arg(quadruple->result) << ", " << powerOfTwo(constant) << std::endl;
                            
                            written.insert(quadruple->result);
                            
                            return;
                        }
                    }
                    
                    spills(Register::EDX);
                    registers.reserve(Register::EDX);
                    
                    //Form x = x / y
                    if(*quadruple->arg1 == quadruple->result){
                        safeMove(quadruple->result, Register::EAX);

                        div(quadruple);
                    //Form x = y / z (y: variable)
                    } else if(isVariable(*quadruple->arg1)){
                        spills(Register::EAX);
                        registers.reserve(Register::EAX);
                        
                        copy(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), Register::EAX);
                        
                        div(quadruple);

                        registers.release(Register::EAX);
                        registers.setLocation(quadruple->result, Register::EAX);
                    } else {
                        spills(Register::EAX);
                        registers.reserve(Register::EAX);
                        
                        copy(*quadruple->arg1, Register::EAX);
                        
                        div(quadruple);

                        registers.release(Register::EAX);
                        registers.setLocation(quadruple->result, Register::EAX);
                    }
                    
                    registers.release(Register::EDX);
                            
                    written.insert(quadruple->result);
                    
                    break;            
                case tac::Operator::MOD:
                    spills(Register::EAX);
                    spills(Register::EDX);

                    registers.reserve(Register::EAX);
                    registers.reserve(Register::EDX);
                    
                    copy(*quadruple->arg1, Register::EAX);

                    div(quadruple);

                    //result is in edx (no need to move it now)
                    registers.setLocation(quadruple->result, Register::EDX);

                    registers.release(Register::EAX);
                    
                    written.insert(quadruple->result);

                    break;            
                case tac::Operator::MINUS:
                {
                    //If arg is immediate, we have to move it in a register
                    if(isInt(*quadruple->arg1)){
                        auto reg = getReg();

                        move(*quadruple->arg1, reg);
                        writer.stream() << "neg " << reg << std::endl;

                        if(registers.reserved(reg)){
                            registers.release(reg);
                        }
                    } else {
                        writer.stream() << "neg " << arg(*quadruple->arg1) << std::endl;
                    }
                    
                    written.insert(quadruple->result);

                    break;
                }
                case tac::Operator::GREATER:
                    setIfCc("setg", quadruple);
                    break;
                case tac::Operator::GREATER_EQUALS:
                    setIfCc("setge", quadruple);
                    break;
                case tac::Operator::LESS:
                    setIfCc("setl", quadruple);
                    break;
                case tac::Operator::LESS_EQUALS:
                    setIfCc("setle", quadruple);
                    break;
                case tac::Operator::EQUALS:
                    setIfCc("sete", quadruple);
                    break;
                case tac::Operator::NOT_EQUALS:
                    setIfCc("setne", quadruple);
                    break;
                case tac::Operator::DOT:
                {
                   assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));
                   assert(boost::get<int>(&*quadruple->arg2));

                   int offset = boost::get<int>(*quadruple->arg2);
                   auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
   
                   Register reg = getRegNoMove(quadruple->result);
                   writer.stream() << "mov " << reg << ", " << toString(variable, offset) << std::endl;
        
                   written.insert(quadruple->result);

                   break;
                }
                case tac::Operator::DOT_ASSIGN:
                {
                    assert(boost::get<int>(&*quadruple->arg1));

                    int offset = boost::get<int>(*quadruple->arg1);

                    writer.stream() << "mov dword " << toString(quadruple->result, offset) << ", " << arg(*quadruple->arg2) << std::endl;

                    break;
                }
                case tac::Operator::ARRAY:
                {
                    assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));

                    Register reg = getRegNoMove(quadruple->result);

                    writer.stream() << "mov " << reg << ", " << toString(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), *quadruple->arg2) << std::endl;
                   
                    written.insert(quadruple->result);
                    
                    break;            
                }
                case tac::Operator::ARRAY_ASSIGN:
                    writer.stream() << "mov dword " << toString(quadruple->result, *quadruple->arg1) << ", " << arg(*quadruple->arg2) << std::endl;

                    break;
                case tac::Operator::PARAM:
                {
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                        if((*ptr)->type().isArray()){
                            auto position = (*ptr)->position();

                            if(position.isGlobal()){
                                Register reg = getReg();
                                
                                auto offset = size((*ptr)->type().base()) * (*ptr)->type().size();

                                writer.stream() << "mov " << reg << ", V" << position.name() << std::endl;
                                writer.stream() << "add " << reg << ", " << offset << std::endl;
                                writer.stream() << "push " << reg << std::endl;

                                registers.release(reg);
                            } else if(position.isStack()){
                                Register reg = getReg();

                                writer.stream() << "mov " << reg << ", ebp" << std::endl;
                                writer.stream() << "add " << reg << ", " << -position.offset() << std::endl;
                                writer.stream() << "push " << reg << std::endl;
                                
                                registers.release(reg);
                            } else if(position.isParameter()){
                                writer.stream() << "push dword [ebp + " << position.offset() << "]" << std::endl;
                            }
                        } else {
                            writer.stream() << "push " << arg(*quadruple->arg1) << std::endl;
                        }
                    } else {
                        writer.stream() << "push " << arg(*quadruple->arg1) << std::endl;
                    }

                    break;
                }
                case tac::Operator::RETURN:
                {
                    //A return without args is the same as exiting from the function
                    if(quadruple->arg1){
                        spillsIfNecessary(Register::EAX, *quadruple->arg1);

                        bool necessary = true;
                        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                            if(registers.inRegister(*ptr, Register::EAX)){
                                necessary = false;
                            }
                        }    

                        if(necessary){
                            writer.stream() << "mov eax, " << arg(*quadruple->arg1) << std::endl;
                        }

                        if(quadruple->arg2){
                            spillsIfNecessary(Register::EBX, *quadruple->arg2);

                            necessary = true;
                            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                                if(registers.inRegister(*ptr, Register::EBX)){
                                    necessary = false;
                                }
                            }    

                            if(necessary){
                                writer.stream() << "mov ebx, " << arg(*quadruple->arg2) << std::endl;
                            }
                        }
                    }
        
                    if(function->context->size() > 0){
                        writer.stream() << "add esp, " << function->context->size() << std::endl;
                    }

                    //The basic block must be ended before the jump
                    endBasicBlock();

                    writer.stream() << "leave" << std::endl;
                    writer.stream() << "ret" << std::endl;

                    break;
                }
            }
        }
    }
    
    template<typename T>
    void compareBinary(T& if_){
        //The first argument is not important, it can be immediate, but the second must be a register
        if(auto* ptr = boost::get<int>(&if_->arg1)){
            auto reg = getReg();

            writer.stream() << "mov " << reg << ", " << *ptr << std::endl;

            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "cmp " << reg << ", " << arg(*if_->arg2) << std::endl;

            registers.release(reg);
        } else {
            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "cmp " << arg(if_->arg1) << ", " << arg(*if_->arg2) << std::endl;
        }
    }

    template<typename T>
    void compareUnary(T& if_){
        if(auto* ptr = boost::get<int>(&if_->arg1)){
            auto reg = getReg();

            writer.stream() << "mov " << reg << "," << *ptr << std::endl;

            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "or " << reg << ", " << reg << std::endl;

            registers.release(reg);
        } else {
            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "or " << arg(if_->arg1) << ", " << arg(if_->arg1) << std::endl;
        }
    }
    
    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        current = ifFalse;

        if(ifFalse->op){
            compareBinary(ifFalse);

            switch(*ifFalse->op){
                case tac::BinaryOperator::EQUALS:
                    writer.stream() << "jne " << labels[ifFalse->block] << std::endl;
                    break;
                case tac::BinaryOperator::NOT_EQUALS:
                    writer.stream() << "je " << labels[ifFalse->block] << std::endl;
                    break;
                case tac::BinaryOperator::LESS:
                    writer.stream() << "jge " << labels[ifFalse->block] << std::endl;
                    break;
                case tac::BinaryOperator::LESS_EQUALS:
                    writer.stream() << "jg " << labels[ifFalse->block] << std::endl;
                    break;
                case tac::BinaryOperator::GREATER:
                    writer.stream() << "jle " << labels[ifFalse->block] << std::endl;
                    break;
                case tac::BinaryOperator::GREATER_EQUALS:
                    writer.stream() << "jl " << labels[ifFalse->block] << std::endl;
                    break;
            }
        } else {
            compareUnary(ifFalse);

            writer.stream() << "jz " << labels[ifFalse->block] << std::endl;
        }
    }

    void operator()(std::shared_ptr<tac::If>& if_){
        current = if_;

        if(if_->op){
            compareBinary(if_);

            switch(*if_->op){
                case tac::BinaryOperator::EQUALS:
                    writer.stream() << "je " << labels[if_->block] << std::endl;
                    break;
                case tac::BinaryOperator::NOT_EQUALS:
                    writer.stream() << "jne " << labels[if_->block] << std::endl;
                    break;
                case tac::BinaryOperator::LESS:
                    writer.stream() << "jl " << labels[if_->block] << std::endl;
                    break;
                case tac::BinaryOperator::LESS_EQUALS:
                    writer.stream() << "jle " << labels[if_->block] << std::endl;
                    break;
                case tac::BinaryOperator::GREATER:
                    writer.stream() << "jg " << labels[if_->block] << std::endl;
                    break;
                case tac::BinaryOperator::GREATER_EQUALS:
                    writer.stream() << "jge " << labels[if_->block] << std::endl;
                    break;
            }
        } else {
            compareUnary(if_);

            writer.stream() << "jnz " << labels[if_->block] << std::endl;
        }
    }

    void operator()(tac::NoOp&){
        //It's a no-op
    }

    void operator()(std::string&){
        assert(false); //There is no more label after the basic blocks have been extracted
    }
};

void as::IntelX86CodeGenerator::compile(std::shared_ptr<tac::Function> function){
    defineFunction(writer, function->getName());

    auto size = function->context->size();
    //Only if necessary, allocates size on the stack for the local variables
    if(size > 0){
        writer.stream() << "sub esp, " << size << std::endl;
    }
    
    auto iter = function->context->begin();
    auto end = function->context->end();

    for(; iter != end; iter++){
        auto var = iter->second;
        if(var->type().isArray() && var->position().isStack()){
            int position = -var->position().offset();

            writer.stream() << "mov dword [ebp + " << position << "], " << var->type().size() << std::endl;

            if(var->type().base() == BaseType::INT){
                writer.stream() << "mov ecx, " << var->type().size() << std::endl;
            } else if(var->type().base() == BaseType::STRING){
                writer.stream() << "mov ecx, " << (var->type().size() * 2) << std::endl;
            }
            
            writer.stream() << "mov eax, 0" << std::endl;
            writer.stream() << "lea edi, [ebp + " << position << " - 4]" << std::endl;
            writer.stream() << "std" << std::endl;
            writer.stream() << "rep stosd" << std::endl;
            writer.stream() << "cld" << std::endl;
        }
    }

    StatementCompiler compiler(writer, function);

    tac::computeBlockUsage(function, compiler.blockUsage);

    //First we computes a label for each basic block
    for(auto& block : function->getBasicBlocks()){
        compiler.labels[block] = newLabel();
    }

    //Then we compile each of them
    for(auto& block : function->getBasicBlocks()){
        compile(block, compiler);
    }
    
    //Only if necessary, deallocates size on the stack for the local variables
    if(size > 0){
        writer.stream() << "add esp, " << size << std::endl;
    }
   
    leaveFunction(writer); 
}

void as::IntelX86CodeGenerator::compile(std::shared_ptr<tac::BasicBlock> block, StatementCompiler& compiler){
    compiler.reset();

    if(compiler.blockUsage.find(block) != compiler.blockUsage.end()){
        writer.stream() << compiler.labels[block] << ":" << std::endl;
    }

    for(unsigned int i = 0; i < block->statements.size(); ++i){
        auto& statement = block->statements[i];

        if(i == block->statements.size() - 1){
            compiler.setLast(true);
        } else {
            compiler.setNext(block->statements[i+1]);
        }
        
        visit(compiler, statement);
    }

    //If the basic block has not been ended
    if(!compiler.ended){
        compiler.endBasicBlock();
    }
}

void IntelX86CodeGenerator::writeRuntimeSupport(FunctionTable& table){
    writer.stream() << "section .text" << std::endl << std::endl;

    writer.stream() << "global _start" << std::endl << std::endl;

    writer.stream() << "_start:" << std::endl;

    //If the user wants the args, we add support for them
    if(table.getFunction("main")->parameters.size() == 1){
        writer.stream() << "pop ebx" << std::endl;                          //ebx = number of args
        writer.stream() << "lea ecx, [4 + ebx * 8]" << std::endl;           //ecx = size of the array
        writer.stream() << "push ecx" << std::endl;
        writer.stream() << "call eddi_alloc" << std::endl;                  //eax = start address of the array
        writer.stream() << "add esp, 4" << std::endl;

        writer.stream() << "lea esi, [eax + ecx - 4]" << std::endl;         //esi = last address of the array
        writer.stream() << "mov edx, esi" << std::endl;                     //edx = last address of the array
        
        writer.stream() << "mov [esi], ebx" << std::endl;                   //Set the length of the array
        writer.stream() << "sub esi, 8" << std::endl;                       //Move to the destination address of the first arg

        writer.stream() << ".copy_args:" << std::endl;
        writer.stream() << "pop edi" << std::endl;                          //edi = address of current args
        writer.stream() << "mov [esi+4], edi" << std::endl;                 //set the address of the string

        /* Calculate the length of the string  */
        writer.stream() << "xor eax, eax" << std::endl;
        writer.stream() << "xor ecx, ecx" << std::endl;
        writer.stream() << "not ecx" << std::endl;
        writer.stream() << "repne scasb" << std::endl;
        writer.stream() << "not ecx" << std::endl;
        writer.stream() << "dec ecx" << std::endl;
        /* End of the calculation */

        writer.stream() << "mov dword [esi], ecx" << std::endl;               //set the length of the string
        writer.stream() << "sub esi, 8" << std::endl;
        writer.stream() << "dec ebx" << std::endl;
        writer.stream() << "jnz .copy_args" << std::endl;

        writer.stream() << "push edx" << std::endl;
    }

    writer.stream() << "call main" << std::endl;
    writer.stream() << "mov eax, 1" << std::endl;
    writer.stream() << "xor ebx, ebx" << std::endl;
    writer.stream() << "int 80h" << std::endl;
}

void IntelX86CodeGenerator::defineDataSection(){
    writer.stream() << std::endl << "section .data" << std::endl;
}

void IntelX86CodeGenerator::declareIntArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "%rep " << size << std::endl;
    writer.stream() << "dd 0" << std::endl;
    writer.stream() << "%endrep" << std::endl;
    writer.stream() << "dd " << size << std::endl;
}

void IntelX86CodeGenerator::declareStringArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "%rep " << size << std::endl;
    writer.stream() << "dd S3" << std::endl;
    writer.stream() << "dd 0" << std::endl;
    writer.stream() << "%endrep" << std::endl;
    writer.stream() << "dd " << size << std::endl;
}

void IntelX86CodeGenerator::declareIntVariable(const std::string& name, int value){
    writer.stream() << "V" << name << " dd " << value << std::endl;
}

void IntelX86CodeGenerator::declareStringVariable(const std::string& name, const std::string& label, int size){
    writer.stream() << "V" << name << " dd " << label << ", " << size << std::endl;
}

void IntelX86CodeGenerator::declareString(const std::string& label, const std::string& value){
    writer.stream() << label << " dd " << value << std::endl;
}

void addPrintIntegerBody(AssemblyFileWriter& writer){
    writer.stream() << "mov eax, [ebp+8]" << std::endl;
    writer.stream() << "xor esi, esi" << std::endl;

    //If the number is negative, we print the - and then the number
    writer.stream() << "cmp eax, 0" << std::endl;
    writer.stream() << "jge .loop" << std::endl;

    writer.stream() << "neg eax" << std::endl;
    writer.stream() << "push eax" << std::endl; //We push eax to not loose it from print_string

    //Print "-" 
    writer.stream() << "push S2" << std::endl;
    writer.stream() << "push 1" << std::endl;
    writer.stream() << "call _F5printS" << std::endl;
    writer.stream() << "add esp, 8" << std::endl;

    //Get the the valueof eax again
    writer.stream() << "pop eax" << std::endl;

    writer.stream() << ".loop" << ":" << std::endl;
    writer.stream() << "mov edx, 0" << std::endl;
    writer.stream() << "mov ebx, 10" << std::endl;
    writer.stream() << "div ebx" << std::endl;
    writer.stream() << "add edx, 48" << std::endl;
    writer.stream() << "push edx" << std::endl;
    writer.stream() << "inc esi" << std::endl;
    writer.stream() << "cmp eax, 0" << std::endl;
    writer.stream() << "jz .next" << std::endl;
    writer.stream() << "jmp .loop" << std::endl;

    writer.stream() << ".next" << ":" << std::endl;
    writer.stream() << "cmp esi, 0" << std::endl;
    writer.stream() << "jz .exit" << std::endl;
    writer.stream() << "dec esi" << std::endl;

    writer.stream() << "mov eax, 4" << std::endl;
    writer.stream() << "mov ecx, esp" << std::endl;
    writer.stream() << "mov ebx, 1" << std::endl;
    writer.stream() << "mov edx, 1" << std::endl;
    writer.stream() << "int 80h" << std::endl;

    writer.stream() << "add esp, 4" << std::endl;

    writer.stream() << "jmp .next" << std::endl;

    writer.stream() << ".exit" << ":" << std::endl;
}

void save(AssemblyFileWriter& writer, const std::vector<std::string>& registers){
    for(auto& reg : registers){
        writer.stream() << "push " << reg << std::endl;
    }
}

void restore(AssemblyFileWriter& writer, const std::vector<std::string>& registers){
    auto it = registers.rbegin();
    auto end = registers.rend();

    while(it != end){
        writer.stream() << "pop " << *it << std::endl;
        ++it;
    }
}

void addPrintIntegerFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printI");

    save(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    addPrintIntegerBody(writer);

    restore(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnI");

    save(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    addPrintIntegerBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    restore(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    leaveFunction(writer);
}

void addPrintBoolBody(AssemblyFileWriter& writer){
    writer.stream() << "mov eax, [ebp-4] " << std::endl;
    writer.stream() << "or eax, eax" << std::endl;
    writer.stream() << "jne .true_print" << std::endl;
    writer.stream() << "push 0" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << "jmp .end" << std::endl;
    writer.stream() << ".true_print:" << std::endl;
    writer.stream() << "push 1" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << ".end:" << std::endl;
}

void addPrintBoolFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printB");

    save(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    addPrintBoolBody(writer);

    restore(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnB");

    save(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    addPrintBoolBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    restore(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    leaveFunction(writer);
}

void addPrintLineFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F7println");

    writer.stream() << "push S1" << std::endl;
    writer.stream() << "push 1" << std::endl;
    writer.stream() << "call _F5printS" << std::endl;
    writer.stream() << "add esp, 8" << std::endl;

    leaveFunction(writer);
}

void addPrintStringBody(AssemblyFileWriter& writer){
    writer.stream() << "mov esi, 0" << std::endl;

    writer.stream() << "mov eax, 4" << std::endl;
    writer.stream() << "mov ebx, 1" << std::endl;
    writer.stream() << "mov ecx, [ebp + 12]" << std::endl;
    writer.stream() << "mov edx, [ebp + 8]" << std::endl;
    writer.stream() << "int 80h" << std::endl;
}

void addPrintStringFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printS");
    
    save(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    addPrintStringBody(writer);

    restore(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnS");
    
    save(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    addPrintStringBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    restore(writer, {"eax", "ebx", "ecx", "edx", "esi"});

    leaveFunction(writer);
}

void addConcatFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "concat");

    writer.stream() << "mov edx, [ebp + 16]" << std::endl;
    writer.stream() << "mov ecx, [ebp + 8]" << std::endl;
    writer.stream() << "add edx, ecx" << std::endl;

    writer.stream() << "push edx" << std::endl;
    writer.stream() << "call eddi_alloc" << std::endl;
    writer.stream() << "add esp, 4" << std::endl;

    writer.stream() << "mov [ebp - 4], eax" << std::endl;
    writer.stream() << "mov ecx, eax" << std::endl;
    writer.stream() << "mov eax, 0" << std::endl;

    writer.stream() << "mov ebx, [ebp + 16]" << std::endl;
    writer.stream() << "mov edx, [ebp + 20]" << std::endl;

    writer.stream() << ".copy_concat_1:" << std::endl;
    writer.stream() << "cmp ebx, 0" << std::endl;
    writer.stream() << "je .end_concat_1"  << std::endl;
    writer.stream() << "mov byte al, [edx]" << std::endl;
    writer.stream() << "mov byte [ecx], al" << std::endl;
    writer.stream() << "add ecx, 1" << std::endl;
    writer.stream() << "add edx, 1" << std::endl;
    writer.stream() << "sub ebx, 1" << std::endl;
    writer.stream() << "jmp .copy_concat_1" << std::endl;
    writer.stream() << ".end_concat_1" << ":" << std::endl;

    writer.stream() << "mov ebx, [ebp + 8]" << std::endl;
    writer.stream() << "mov edx, [ebp + 12]" << std::endl;

    writer.stream() << ".copy_concat_2:" << std::endl;
    writer.stream() << "cmp ebx, 0" << std::endl;
    writer.stream() << "je .end_concat_2"  << std::endl;
    writer.stream() << "mov byte al, [edx]" << std::endl;
    writer.stream() << "mov byte [ecx], al" << std::endl;
    writer.stream() << "add ecx, 1" << std::endl;
    writer.stream() << "add edx, 1" << std::endl;
    writer.stream() << "sub ebx, 1" << std::endl;
    writer.stream() << "jmp .copy_concat_2" << std::endl;
    writer.stream() << ".end_concat_2:" << std::endl;

    writer.stream() << "mov ebx, [ebp + 16]" << std::endl;
    writer.stream() << "mov ecx, [ebp + 8]" << std::endl;
    writer.stream() << "add ebx, ecx" << std::endl;

    writer.stream() << "mov eax, [ebp - 4]" << std::endl;

    leaveFunction(writer);
}

void addAllocFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "eddi_alloc");

    save(writer, {"ebx", "ecx", "edx"});

    writer.stream() << "mov ecx, [ebp + 8]" << std::endl;
    writer.stream() << "mov ebx, [Veddi_remaining]" << std::endl;

    writer.stream() << "cmp ecx, ebx" << std::endl;
    writer.stream() << "jle .alloc_normal" << std::endl;

    //Get the current address
    writer.stream() << "mov eax, 45" << std::endl;          //45 = sys_brk
    writer.stream() << "xor ebx, ebx" << std::endl;         //get end
    writer.stream() << "int 80h" << std::endl;

    //%eax is the current address 
    writer.stream() << "mov esi, eax" << std::endl;

    //Alloc new block of 16384K from the current address
    writer.stream() << "mov ebx, eax" << std::endl;
    writer.stream() << "add ebx, 16384" << std::endl;
    writer.stream() << "mov eax, 45" << std::endl;          //45 = sys_brk
    writer.stream() << "int 80h" << std::endl;

    //zero'd the new block
    writer.stream() << "mov edi, eax" << std::endl;         //edi = start of block

    writer.stream() << "sub edi, 4" << std::endl;           //edi points to the last DWORD available to us
    writer.stream() << "mov ecx, 4096" << std::endl;        //this many DWORDs were allocated
    writer.stream() << "xor eax, eax"  << std::endl;        //will write with zeroes
    writer.stream() << "std"  << std::endl;                 //walk backwards
    writer.stream() << "rep stosb"  << std::endl;           //write all over the reserved area
    writer.stream() << "cld"  << std::endl;                 //bring back the DF flag to normal state

    writer.stream() << "mov eax, esi" << std::endl;

    //We now have 16K of available memory starting at %esi
    writer.stream() << "mov dword [Veddi_remaining], 16384" << std::endl;
    writer.stream() << "mov [Veddi_current], esi" << std::endl;

    writer.stream() << ".alloc_normal:" << std::endl;

    //old = current
    writer.stream() << "mov eax, [Veddi_current]" << std::endl;

    //current += size
    writer.stream() << "mov ebx, [Veddi_current]" << std::endl;
    writer.stream() << "add ebx, ecx" << std::endl;
    writer.stream() << "mov [Veddi_current], ebx" << std::endl;

    //remaining -= size
    writer.stream() << "mov ebx, [Veddi_remaining]" << std::endl;
    writer.stream() << "sub ebx, ecx" << std::endl;
    writer.stream() << "mov [Veddi_remaining], ebx" << std::endl;

    writer.stream() << ".alloc_end:" << std::endl;

    restore(writer, {"ebx", "ecx", "edx"});

    leaveFunction(writer);
}

void addTimeFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F4timeAI");

    writer.stream() << "xor eax, eax" << std::endl;
    writer.stream() << "cpuid" << std::endl;                //only to serialize instruction stream
    writer.stream() << "rdtsc" << std::endl;                //edx:eax = timestamp

    writer.stream() << "mov esi, [ebp + 8]" << std::endl;
    writer.stream() << "mov [esi - 4], eax" << std::endl;
    writer.stream() << "mov [esi - 8], edx" << std::endl;

    leaveFunction(writer);
}

void addDurationFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F8durationAIAI");

    writer.stream() << "mov esi, [ebp + 12]" << std::endl;          //Start time stamp
    writer.stream() << "mov edi, [ebp + 8]" << std::endl;           //End time stamp

    //Print the high order bytes
    writer.stream() << "mov eax, [esi - 8]" << std::endl;
    writer.stream() << "mov ebx, [edi - 8]" << std::endl;
    writer.stream() << "sub eax, ebx" << std::endl;
   
    //if the first diff is 0, do not print 0
    writer.stream() << "cmp eax, 0" << std::endl;
    writer.stream() << "jz .second" << std::endl;

    //If it's negative, we print the positive only 
    writer.stream() << "cmp eax, 0" << std::endl;
    writer.stream() << "jge .push_first" << std::endl;
    writer.stream() << "neg eax" << std::endl;
    
    writer.stream() << ".push_first:" << std::endl; 
    writer.stream() << "push eax" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;

    //Print the low order bytes
    writer.stream() << ".second:" << std::endl;
    writer.stream() << "mov eax, [esi - 4]" << std::endl;
    writer.stream() << "mov ebx, [edi - 4]" << std::endl;
    writer.stream() << "sub eax, ebx" << std::endl;
   
    //If it's negative, we print the positive only 
    writer.stream() << "cmp eax, 0" << std::endl;
    writer.stream() << "jge .push_second" << std::endl;
    writer.stream() << "neg eax" << std::endl;
   
    writer.stream() << ".push_second:" << std::endl; 
    writer.stream() << "push eax" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;

    leaveFunction(writer);
}

}} //end of eddic::as

void as::IntelX86CodeGenerator::addStandardFunctions(){
   addPrintIntegerFunction(writer); 
   addPrintBoolFunction(writer);
   addPrintLineFunction(writer); 
   addPrintStringFunction(writer); 
   addConcatFunction(writer);
   addAllocFunction(writer);
   addTimeFunction(writer);
   addDurationFunction(writer);
}
