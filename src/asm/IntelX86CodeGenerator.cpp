//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <boost/variant.hpp>

#include "asm/IntelX86CodeGenerator.hpp"
#include "asm/Registers.hpp"

#include "tac/Printer.hpp"
#include "tac/Utils.hpp"

#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "StringPool.hpp"
#include "Labels.hpp"
#include "VisitorUtils.hpp"

using namespace eddic;

as::IntelX86CodeGenerator::IntelX86CodeGenerator(AssemblyFileWriter& w) : writer(w) {}

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
    switch(reg){
        case EAX:
            return "eax";
        case EBX:
            return "ebx";
        case ECX:
            return "ecx";
        case EDX:
            return "edx";
        case ESP:
            return "esp";
        case EBP:
            return "ebp";
        case ESI:
            return "esi";
        case EDI:
            return "edi";
        default:
            assert(false); //Not a register
    }
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

struct StatementCompiler : public boost::static_visitor<> {
    AssemblyFileWriter& writer;
    std::shared_ptr<tac::Function> function;

    std::unordered_map<std::shared_ptr<tac::BasicBlock>, std::string> labels;
    std::unordered_set<std::shared_ptr<tac::BasicBlock>> blockUsage;

    Registers<Register> registers;

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
                
                writer.stream() << "mov " << reg << ", " << oldReg << std::endl;

                //There is nothing more in the old register
                registers.remove(variable);
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

    void spills(Register reg){
        //If the register is not used, there is nothing to spills
        if(registers.used(reg)){
            auto variable = registers[reg];
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
            
            //The variable is no more contained in the register
            registers.remove(variable);
        }
    }
    
    Register getReg(std::shared_ptr<Variable> variable, bool doMove){
        //The variable is already in a register
        if(registers.inRegister(variable)){
            return registers[variable];
        }
       
        //Try to get a free register 
        for(auto reg : registers){
            if(!registers.used(reg)){
                if(doMove){
                    move(variable, reg);
                }
                
                registers.setLocation(variable, reg);
                
                return reg;
            } else if(!registers.reserved(reg) && !isLive(registers[reg])){
                registers.remove(registers[reg]);
                
                if(doMove){
                    move(variable, reg);
                }

                registers.setLocation(variable, reg);

                return reg;
            }
        }

        //There are no free register, take one
        auto reg = registers.first();
        bool found = false;
        for(auto remaining : registers){
            if(!registers.reserved(remaining)){
                reg = remaining;
                found = true;
            }
        }

        assert(found);
        spills(reg);

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
        //Try to get a free register 
        for(auto reg : registers){
            if(!registers.used(reg)){
                registers.reserve(reg);
                
                return reg;
            } else if(!registers.reserved(reg) && !isLive(registers[reg])){
                registers.remove(registers[reg]);

                registers.reserve(reg);

                return reg;
            }
        }

        //There are no free register, take one
        auto reg = registers.first();
        spills(reg);

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

                writer.stream() << "mov " << reg << "[ebp + " << ::toString(position.offset()) << "]" << std::endl;

                registers.release(reg);

                return "[" + reg + "+" + ::toString(offset) + "]" + ")";
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
            return "[" + offsetReg + "+" + position.name() + "]";
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
        }

        if(call->return2_){
            registers.setLocation(call->return2_, Register::EBX);
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
        
        static int ctr = 0;
        ++ctr;

        //TODO In the future avoid that to allow any value other than 0 as true
        writer.stream() << "or " << reg << ", " << reg << std::endl;
        writer.stream() << "jz " << "intern" << ctr << std::endl;
        writer.stream() << "mov " << reg << ", 1" << std::endl;
        writer.stream() << "intern" << ctr << ":" << std::endl;
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
        } else {
            switch(*quadruple->op){
                case tac::Operator::ADD:
                {
                    auto result = quadruple->result;

                    //Optimize the special form a = a + b by using only one instruction
                    if(tac::equals<std::shared_ptr<Variable>>(*quadruple->arg1, result)){
                        Register reg = getReg(quadruple->result);
                        
                        //a = a + 1 => increment a
                        if(tac::equals<int>(*quadruple->arg2, 1)){
                            writer.stream() << "inc " << reg << std::endl;
                        }
                        //a = a + -1 => decrement a
                        else if(tac::equals<int>(*quadruple->arg2, -1)){
                            writer.stream() << "dec " << reg << std::endl;
                        }
                        //In the other cases, perform a simple addition
                        else {
                            writer.stream() << "add " << reg << ", " << arg(*quadruple->arg2) << std::endl;
                        }
                    } 
                    //Optimize the special form a = b + a by using only one instruction
                    else if(tac::equals<std::shared_ptr<Variable>>(*quadruple->arg2, result)){
                        Register reg = getReg(quadruple->result);
                        
                        //a = 1 + a => increment a
                        if(tac::equals<int>(*quadruple->arg1, 1)){
                            writer.stream() << "inc " << reg << std::endl;
                        }
                        //a = -1 + a => decrement a
                        else if(tac::equals<int>(*quadruple->arg1, -1)){
                            writer.stream() << "dec " << reg << std::endl;
                        }
                        //In the other cases, perform a simple addition
                        else {
                            writer.stream() << "add " << reg << ", " << arg(*quadruple->arg1) << std::endl;
                        }
                    } 
                    //In the other cases, move the first arg into the result register and then add the second arg into it
                    else {
                        Register reg = getRegNoMove(quadruple->result);
                        writer.stream() << "mov " << reg << ", " << arg(*quadruple->arg1) << std::endl;
                        writer.stream() << "add " << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }

                    break;
                }
                case tac::Operator::SUB:
                {
                    auto result = quadruple->result;
                    
                    //Optimize the special form a = a - b by using only one instruction
                    if(tac::equals<std::shared_ptr<Variable>>(*quadruple->arg1, result)){
                        Register reg = getReg(quadruple->result);
                        
                        //a = a - 1 => decrement a
                        if(tac::equals<int>(*quadruple->arg2, 1)){
                            writer.stream() << "dec " << reg << std::endl;
                        }
                        //a = a - -1 => increment a
                        else if(tac::equals<int>(*quadruple->arg2, -1)){
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
                    
                    break;
                }
                //TODO Simplify this generation
                case tac::Operator::MUL:
                {
                    bool fast = false;

                    //Form x = -1 * x
                    if(tac::equals<int>(*quadruple->arg1, -1) && tac::equals<std::shared_ptr<Variable>>(*quadruple->arg2, quadruple->result)){
                        writer.stream() << "neg " << arg(quadruple->result) << std::endl;

                        fast = true;
                    } 
                    //Form x = x * -1
                    else if(tac::equals<int>(*quadruple->arg2, -1) && tac::equals<std::shared_ptr<Variable>>(*quadruple->arg1, quadruple->result)){
                        writer.stream() << "neg " << arg(quadruple->result) << std::endl;

                        fast = true;
                    }

                    //If arg 1 is in eax
                    if(!fast){
                        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                            if(registers.inRegister(*ptr, Register::EAX)){
                                if((*ptr)->position().isTemporary() && !isNextLive(*ptr)){
                                    //If the arg is a variable, it will be matched to a register automatically
                                    if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2))
                                    {
                                        writer.stream() << "mul " << arg(*quadruple->arg2) << std::endl;
                                    } //If it's an immediate value, we have to move it in a register
                                    else if (boost::get<int>(&*quadruple->arg2)){
                                        auto reg = getReg();

                                        move(*quadruple->arg2, reg);
                                        writer.stream() << "mul " << reg << std::endl;

                                        if(registers.reserved(reg)){
                                            registers.release(reg);
                                        }
                                    }

                                    fast = true;
                                }
                            }
                        }
                    }

                    //If arg 2 is in eax
                    if(!fast){
                        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                            if(registers.inRegister(*ptr, Register::EAX)){
                                if((*ptr)->position().isTemporary() && !isNextLive(*ptr)){
                                    //If the arg is a variable, it will be matched to a register automatically
                                    if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1))
                                    {
                                        writer.stream() << "mul " << arg(*quadruple->arg1) << std::endl;
                                    } //If it's an immediate value, we have to move it in a register
                                    else if (boost::get<int>(&*quadruple->arg1)){
                                        auto reg = getReg();

                                        move(*quadruple->arg1, reg);
                                        writer.stream() << "mul " << reg << std::endl;

                                        if(registers.reserved(reg)){
                                            registers.release(reg);
                                        }
                                    }

                                    fast = true;
                                }
                            }
                        }
                    }

                    //Neither of the args is in a register
                    if(!fast){
                        spills(Register::EAX);

                        registers.reserve(Register::EAX);
                        copy(*quadruple->arg1, Register::EAX);
                        
                        //If the arg is a variable, it will be matched to a register automatically
                        if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2))
                        {
                            writer.stream() << "mul " << arg(*quadruple->arg2) << std::endl;
                        } //If it's an immediate value, we have to move it in a register
                        else if (boost::get<int>(&*quadruple->arg2)){
                            auto reg = getReg();

                            move(*quadruple->arg2, reg);
                            writer.stream() << "mul " << reg << std::endl;

                            if(registers.reserved(reg)){
                                registers.release(reg);
                            }
                        }
                    }

                    //result is in eax (no need to move it now)
                    registers.setLocation(quadruple->result, Register::EAX);

                    break;            
                }
                case tac::Operator::DIV:
                    spills(Register::EAX);
                    spills(Register::EDX);

                    registers.reserve(Register::EAX);
                    registers.reserve(Register::EDX);
                    
                    copy(*quadruple->arg1, Register::EAX);
                    writer.stream() << "xor edx, edx" << std::endl;

                    //If the second arg is immediate, we have to move it in a register
                    if(boost::get<int>(&*quadruple->arg2)){
                        auto reg = getReg();

                        move(*quadruple->arg2, reg);
                        writer.stream() << "div " << reg << std::endl;

                        if(registers.reserved(reg)){
                            registers.release(reg);
                        }
                    } else {
                        writer.stream() << "div " << arg(*quadruple->arg2) << std::endl;
                    }

                    //result is in eax (no need to move it now)
                    registers.setLocation(quadruple->result, Register::EAX);
                    
                    registers.release(Register::EDX);
                    
                    break;            
                case tac::Operator::MOD:
                    spills(Register::EAX);
                    spills(Register::EDX);

                    registers.reserve(Register::EAX);
                    registers.reserve(Register::EDX);
                    
                    copy(*quadruple->arg1, Register::EAX);
                    writer.stream() << "xor edx, edx" << std::endl;

                    //If the second arg is immediate, we have to move it in a register
                    if(boost::get<int>(&*quadruple->arg2)){
                        auto reg = getReg();

                        move(*quadruple->arg2, reg);
                        writer.stream() << "div " << reg << std::endl;

                        if(registers.reserved(reg)){
                            registers.release(reg);
                        }
                    } else {
                        writer.stream() << "div " << arg(*quadruple->arg2) << std::endl;
                    }

                    //result is in edx (no need to move it now)
                    registers.setLocation(quadruple->result, Register::EDX);

                    registers.release(Register::EAX);

                    break;            
                case tac::Operator::MINUS:
                {
                    //If arg is immediate, we have to move it in a register
                    if(boost::get<int>(&*quadruple->arg1)){
                        auto reg = getReg();

                        move(*quadruple->arg1, reg);
                        writer.stream() << "neg " << reg << std::endl;

                        if(registers.reserved(reg)){
                            registers.release(reg);
                        }
                    } else {
                        writer.stream() << "neg " << arg(*quadruple->arg1) << std::endl;
                    }

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

                   break;
                }
                case tac::Operator::DOT_ASSIGN:
                {
                    assert(boost::get<int>(&*quadruple->arg1));

                    int offset = boost::get<int>(*quadruple->arg1);

                    writer.stream() << "mov " << toString(quadruple->result, offset) << ", " << arg(*quadruple->arg2) << std::endl;

                    break;
                }
                case tac::Operator::ARRAY:
                {
                    assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));

                    Register reg = getRegNoMove(quadruple->result);

                    writer.stream() << "mov " << reg << ", " << toString(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), *quadruple->arg2) << std::endl;
                    
                    break;            
                }
                case tac::Operator::ARRAY_ASSIGN:
                    writer.stream() << "mov " << toString(quadruple->result, *quadruple->arg1) << ", " << arg(*quadruple->arg2) << std::endl;

                    break;
                case tac::Operator::PARAM:
                {
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                        if((*ptr)->type().isArray()){
                            auto position = (*ptr)->position();

                            if(position.isGlobal()){
                                Register reg = getReg();
                                
                                auto offset = size((*ptr)->type().base()) * (*ptr)->type().size();

                                writer.stream() << "movl $V" << position.name() << ", " << reg << std::endl;
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
                                writer.stream() << "push [ebp + " << position.offset() << "]" << std::endl;
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

}}

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

void as::IntelX86CodeGenerator::compile(std::shared_ptr<tac::Function> function){
    writer.stream() << std::endl << function->getName() << ":" << std::endl;
    
    writer.stream() << "push ebp" << std::endl;
    writer.stream() << "mov ebp, esp" << std::endl;

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

            writer.stream() << "mov [ebp + " << position << "], " << var->type().size() << std::endl;

            if(var->type().base() == BaseType::INT){
                for(unsigned int i = 0; i < var->type().size(); ++i){
                    writer.stream() << "mov [ebp + " << (position -= 4) << "], 0" << std::endl;
                }
            } else if(var->type().base() == BaseType::STRING){
                for(unsigned int i = 0; i < var->type().size(); ++i){
                    writer.stream() << "mov [ebp + " << (position -= 4) << "], 0" << std::endl;
                    writer.stream() << "mov [ebp + " << (position -= 4) << "], 0" << std::endl;
                }
            }
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
    
    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void as::IntelX86CodeGenerator::writeRuntimeSupport(){
    writer.stream() << "section .text" << std::endl << std::endl;

    writer.stream() << "global _start" << std::endl << std::endl;

    writer.stream() << "_start:" << std::endl;
    writer.stream() << "call main" << std::endl;
    writer.stream() << "mov eax, 1" << std::endl;
    writer.stream() << "xor ebx, ebx" << std::endl;
    writer.stream() << "int 80h" << std::endl;
}

void addPrintIntegerBody(AssemblyFileWriter& writer){
    static int ctr = 0;
    ctr++;

    writer.stream() << "mov eax, [ebp+8]" << std::endl
        << "xor esi, esi" << std::endl

        //If the number is negative, we print the - and then the number
        << "cmp eax, 0" << std::endl
        << "jge loop" << ctr << std::endl

        << "neg eax" << std::endl
        << "push eax" << std::endl //We push eax to not loose it from print_string

        //Print "-" 
        << "push S2" << std::endl
        << "push 1" << std::endl
        << "call _F5printS" << std::endl
        << "add esp, 8" << std::endl

        //Get the the valueof eax again
        << "pop eax" << std::endl

        << "loop" << ctr << ":" << std::endl
        << "mov edx, 0" << std::endl
        << "mov ebx, 10" << std::endl
        << "div ebx" << std::endl
        << "add edx, 48" << std::endl
        << "push edx" << std::endl
        << "inc esi" << std::endl
        << "cmp eax, 0" << std::endl
        << "jz   next" << ctr << std::endl
        << "jmp loop" << ctr << std::endl

        << "next" << ctr << ":" << std::endl
        << "cmp esi, 0" << std::endl
        << "jz   exit" << ctr << std::endl
        << "dec esi" << std::endl

        << "mov eax, 4" << std::endl
        << "mov ecx, esp" << std::endl
        << "mov ebx, 1" << std::endl
        << "mov edx, 1" << std::endl
        << "int 80h" << std::endl

        << "add esp, 4" << std::endl

        << "jmp next" << ctr << std::endl

        << "exit" << ctr << ":" << std::endl;
}

void addPrintIntegerFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "_F5printB:" << std::endl;
    writer.stream() << "_F5printI:" << std::endl;
    writer.stream() << "push ebp" << std::endl;
    writer.stream() << "mov ebp, esp" << std::endl;

    //Save registers
    writer.stream() << "push eax" << std::endl;
    writer.stream() << "push ebx" << std::endl;
    writer.stream() << "push ecx" << std::endl;
    writer.stream() << "push edx" << std::endl;

    addPrintIntegerBody(writer);

    //Restore registers
    writer.stream() << "pop edx" << std::endl;
    writer.stream() << "pop ecx" << std::endl;
    writer.stream() << "pop ebx" << std::endl;
    writer.stream() << "pop eax" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
   
    /* println version */
    
    writer.stream() << std::endl;
    writer.stream() << "_F7printlnB:" << std::endl;
    writer.stream() << "_F7printlnI:" << std::endl;
    writer.stream() << "push ebp" << std::endl;
    writer.stream() << "mov ebp, esp" << std::endl;

    //Save registers
    writer.stream() << "push eax" << std::endl;
    writer.stream() << "push ebx" << std::endl;
    writer.stream() << "push ecx" << std::endl;
    writer.stream() << "push edx" << std::endl;

    addPrintIntegerBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    //Restore registers
    writer.stream() << "pop edx" << std::endl;
    writer.stream() << "pop ecx" << std::endl;
    writer.stream() << "pop ebx" << std::endl;
    writer.stream() << "pop eax" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void addPrintLineFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "_F7println:" << std::endl;
    writer.stream() << "push ebp" << std::endl;
    writer.stream() << "mov ebp, esp" << std::endl;

    writer.stream() << "push S1" << std::endl;
    writer.stream() << "push 1" << std::endl;
    writer.stream() << "call _F5printS" << std::endl;
    writer.stream() << "add esp, 8" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
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
    writer.stream() << std::endl;
    writer.stream() << "_F5printS:" << std::endl;
    writer.stream() << "push ebp" << std::endl;
    writer.stream() << "mov ebp, esp" << std::endl;
    
    //Save registers
    writer.stream() << "push eax" << std::endl;
    writer.stream() << "push ebx" << std::endl;
    writer.stream() << "push ecx" << std::endl;
    writer.stream() << "push edx" << std::endl;

    addPrintStringBody(writer);

    //Restore registers
    writer.stream() << "pop edx" << std::endl;
    writer.stream() << "pop ecx" << std::endl;
    writer.stream() << "pop ebx" << std::endl;
    writer.stream() << "pop eax" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
   
    /* println version */
    
    writer.stream() << std::endl;
    writer.stream() << "_F7printlnS:" << std::endl;
    writer.stream() << "push ebp" << std::endl;
    writer.stream() << "mov ebp, esp" << std::endl;
    
    //Save registers
    writer.stream() << "push eax" << std::endl;
    writer.stream() << "push ebx" << std::endl;
    writer.stream() << "push ecx" << std::endl;
    writer.stream() << "push edx" << std::endl;

    addPrintStringBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    //Restore registers
    writer.stream() << "pop edx" << std::endl;
    writer.stream() << "pop ecx" << std::endl;
    writer.stream() << "pop ebx" << std::endl;
    writer.stream() << "pop eax" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void addConcatFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "concat:" << std::endl
        << "push ebp" << std::endl
        << "mov ebp, esp" << std::endl

        << "mov edx, [ebp + 16]" << std::endl
        << "mov ecx, [ebp + 8]" << std::endl
        << "add edx, ecx" << std::endl

        << "push edx" << std::endl
        << "call eddi_alloc" << std::endl
        << "add esp, 4" << std::endl

        << "mov [ebp - 4], eax" << std::endl
        << "mov ecx, eax" << std::endl
        << "mov eax, 0" << std::endl

        << "mov ebx, [ebp + 16]" << std::endl
        << "mov edx, [ebp + 20]" << std::endl

        << "copy_concat_1:" << std::endl
        << "cmp ebx, 0" << std::endl
        << "je end_concat_1"  << std::endl
        << "mov byte al, [edx]" << std::endl
        << "mov byte [ecx], al" << std::endl
        << "add ecx, 1" << std::endl
        << "add edx, 1" << std::endl
        << "sub ebx, 1" << std::endl
        << "jmp copy_concat_1" << std::endl
        << "end_concat_1" << ":" << std::endl

        << "mov ebx, [ebp + 8]" << std::endl
        << "mov edx, [ebp + 12]" << std::endl

        << "copy_concat_2:" << std::endl
        << "cmp ebx, 0" << std::endl
        << "je end_concat_2"  << std::endl
        << "mov byte al, [edx]" << std::endl
        << "mov byte [ecx], al" << std::endl
        << "add ecx, 1" << std::endl
        << "add edx, 1" << std::endl
        << "sub ebx, 1" << std::endl
        << "jmp copy_concat_2" << std::endl
        << "end_concat_2:" << std::endl

        << "mov ebx, [ebp + 16]" << std::endl
        << "mov ecx, [ebp + 8]" << std::endl
        << "add ebx, ecx" << std::endl

        << "mov eax, [ebp - 4]" << std::endl

        << "leave" << std::endl
        << "ret" << std::endl;
}

void addAllocFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "eddi_alloc:" << std::endl
        << "push ebp" << std::endl
        << "mov ebp, esp" << std::endl

        //Save registers
        << "push ebx" << std::endl
        << "push ecx" << std::endl
        << "push edx" << std::endl

        << "mov ecx, [ebp + 8]" << std::endl
        << "mov ebx, [Veddi_remaining]" << std::endl

        << "cmp ecx, ebx" << std::endl
        << "jle alloc_normal" << std::endl

        //Get the current address
        << "mov eax, 45" << std::endl        //45 = sys_brk
        << "xor ebx, ebx" << std::endl       //get end
        << "int 80h" << std::endl
       
        //%eax is the current address 
        << "mov esi, eax" << std::endl

        //Alloc new block of 16384K from the current address
        << "mov ebx, eax" << std::endl
        << "add ebx, 16384" << std::endl
        << "mov eax, 45" << std::endl        //45 = sys_brk
        << "int 80h" << std::endl

        //zero'd the new block
        << "mov edi, eax" << std::endl       //edi = start of block

        << "sub edi, 4" << std::endl         //EDI points to the last DWORD available to us
        << "mov ecx, 4096" << std::endl         //this many DWORDs were allocated
        << "xor eax, eax"  << std::endl         //   ; will write with zeroes
        << "std"  << std::endl         //        ; walk backwards
        << "rep stosb"  << std::endl         //      ; write all over the reserved area
        << "cld"  << std::endl         //        ; bring back the DF flag to normal state

        << "mov eax, esi" << std::endl

        //We now have 16K of available memory starting at %esi
        << "mov dword [Veddi_remaining], 16384" << std::endl
        << "mov [Veddi_current], esi" << std::endl

        << "alloc_normal:" << std::endl

        //old = current
        << "mov eax, [Veddi_current]" << std::endl
        
        //current += size
        << "mov ebx, [Veddi_current]" << std::endl
        << "add ebx, ecx" << std::endl
        << "mov [Veddi_current], ebx" << std::endl
        
        //remaining -= size
        << "mov ebx, [Veddi_remaining]" << std::endl
        << "sub ebx, ecx" << std::endl
        << "mov [Veddi_remaining], ebx" << std::endl
       
        << "alloc_end:" << std::endl

        //Restore registers
        << "pop edx" << std::endl
        << "pop ecx" << std::endl
        << "pop ebx" << std::endl

        << "leave" << std::endl
        << "ret" << std::endl;
}

void as::IntelX86CodeGenerator::addStandardFunctions(){
   addPrintIntegerFunction(writer); 
   addPrintLineFunction(writer); 
   addPrintStringFunction(writer); 
   addConcatFunction(writer);
   addAllocFunction(writer);
}

void as::IntelX86CodeGenerator::addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool){
    writer.stream() << std::endl << "section .data" << std::endl;
     
    for(auto it : context->getVariables()){
        Type type = it.second->type();
        
        //The const variables are not stored
        if(type.isConst()){
            continue;
        }

        if(type.isArray()){
            writer.stream() << "V" << it.second->position().name() << ":" <<std::endl;
            writer.stream() << "%rep " << type.size() << std::endl;

            if(type.base() == BaseType::INT){
                writer.stream() << "dd 0" << std::endl;
            } else if(type.base() == BaseType::STRING){
                writer.stream() << "dd S3" << std::endl;
                writer.stream() << "dd 0" << std::endl;
            }

            writer.stream() << "%endrep" << std::endl;
            writer.stream() << "dd " << type.size() << std::endl;
        } else {
            if (type.base() == BaseType::INT) {
                writer.stream() << "V" << it.second->position().name() << " dd " << boost::get<int>(it.second->val()) << std::endl;
            } else if (type.base() == BaseType::STRING) {
                auto value = boost::get<std::pair<std::string, int>>(it.second->val());
  
                //If that's not the case, there is a problem with the pool 
                assert(value.first.size() > 0);
                
                writer.stream() << "V" << it.second->position().name() << " dd " << pool.label(value.first) << ", " << value.second << std::endl;
            }
        }
    }
    
    for (auto it : pool.getPool()){
        writer.stream() << it.second << " dd " << it.first  << std::endl;
    }
}

void as::IntelX86CodeGenerator::generate(tac::Program& program, StringPool& pool){
    writeRuntimeSupport(); 

    resetNumbering();

    for(auto& function : program.functions){
        compile(function);
    }

    addStandardFunctions();

    addGlobalVariables(program.context, pool);
}
