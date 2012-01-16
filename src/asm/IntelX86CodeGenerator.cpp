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
            return "%eax";
        case EBX:
            return "%ebx";
        case ECX:
            return "%ecx";
        case EDX:
            return "%edx";
        case ESP:
            return "%esp";
        case EBP:
            return "%ebp";
        case ESI:
            return "%esi";
        case EDI:
            return "%edi";
        default:
            assert(false); //Not a register
    }
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
        } else if(auto* ptr = boost::get<std::shared_ptr<tac::Return>>(&statement)){
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
            writer.stream() << "movl $" << ::toString(*ptr) << ", " << regToString(reg) << std::endl;
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            writer.stream() << "movl $" << *ptr << ", " << regToString(reg) << std::endl;
        } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            auto variable = *ptr;

            //If the variable is hold in a register, just move the register value
            if(registers.inRegister(variable)){
                auto oldReg = registers[variable];
                
                writer.stream() << "movl " << regToString(oldReg) << ", " << regToString(reg) << std::endl;
            } else {
                auto position = variable->position();

                if(position.isStack()){
                    writer.stream() << "movl " << (-1 * position.offset()) << "(%ebp), " << regToString(reg) << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << "movl " << position.offset() << "(%ebp), " << regToString(reg) << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << "movl " << "V" << position.name() << ", " << regToString(reg) << std::endl;
                } else if(position.isTemporary()){
                    //The temporary should have been handled by the preceding condition (hold in a register)
                    assert(false);
                }
            } 
        }
    }

    void move(tac::Argument argument, Register reg){
        if(auto* ptr = boost::get<int>(&argument)){
            writer.stream() << "movl $" << ::toString(*ptr) << ", " << regToString(reg) << std::endl;
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            writer.stream() << "movl $" << *ptr << ", " << regToString(reg) << std::endl;
        } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            auto variable = *ptr;

            //If the variable is hold in a register, just move the register value
            if(registers.inRegister(variable)){
                auto oldReg = registers[variable];
                
                writer.stream() << "movl " << regToString(oldReg) << ", " << regToString(reg) << std::endl;

                //There is nothing more in the old register
                registers.remove(variable);
            } else {
                auto position = variable->position();

                if(position.isStack()){
                    writer.stream() << "movl " << (-1 * position.offset()) << "(%ebp), " << regToString(reg) << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << "movl " << position.offset() << "(%ebp), " << regToString(reg) << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << "movl " << "V" << position.name() << ", " << regToString(reg) << std::endl;
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
                writer.stream() << "movl " << regToString(reg) << ", " << (-1 * position.offset()) << "(%ebp)" << std::endl; 
            } else if(position.isParameter()){
                writer.stream() << "movl " << regToString(reg) << ", " <<  position.offset() << "(%ebp)" << std::endl; 
            } else if(position.isGlobal()){
                writer.stream() << "movl " << regToString(reg) << ", V" << position.name() << std::endl;
            } else if(position.isTemporary()){
                //If the variable is live, move it to another register, else do nothing
                if(isLive(variable)){
                    registers.remove(variable);
                    registers.reserve(reg);
                    
                    Register newReg = getReg(variable, false);
                    writer.stream() << "movl " << regToString(reg) << ", " << regToString(newReg) << std::endl;

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
            return ::toString(-position.offset() + offset) + "(%ebp)";
        } else if(position.isParameter()){
            //The case of array is special because only the address is passed, not the complete array
            if(variable->type().isArray())
            {
                //TODO This register allocation is not safe
                Register reg = getReg();

                writer.stream() << "movl " << ::toString(position.offset()) << "(%ebp), " << regToString(reg) << std::endl;

                registers.release(reg);

                return ::toString(offset) + "(" + regToString(reg)  + ")";
            } 
            //In the other cases, the value is passed, so we can compute the offset directly
            else {
                return ::toString(position.offset() + offset) + "(%ebp)";
            }
        } else if(position.isGlobal()){
            return "V" + position.name() + "+" + ::toString(offset);
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
            return ::toString(-1 * (position.offset())) + "(%ebp, " + regToString(offsetReg) + ",1)";
        } else if(position.isParameter()){
            //TODO This register allocation is not safe
            Register reg = getReg();
            
            writer.stream() << "movl " << ::toString(position.offset()) << "(%ebp), " << regToString(reg) << std::endl;

            registers.release(reg);

            return "(" + regToString(reg) + "," + regToString(offsetReg) + ")";
        } else if(position.isGlobal()){
            return "V" + position.name() + "(" + regToString(offsetReg) + ")";
        } else if(position.isTemporary()){
            assert(false); //We are in da shit
        }

        assert(false);
    }

    std::string arg(tac::Argument argument){
        if(auto* ptr = boost::get<int>(&argument)){
            return "$" + ::toString(*ptr);
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            return "$" + *ptr;
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
            writer.stream() << "addl $" << call->params << ", %esp" << std::endl;
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
   
    //TODO Move the necessary calculation part into another function 
    void operator()(std::shared_ptr<tac::Return>& return_){
        current = return_;

        //A return without args is the same as exiting from the function
        if(return_->arg1){
            spillsIfNecessary(Register::EAX, *return_->arg1);

            bool necessary = true;
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*return_->arg1)){
                if(registers.inRegister(*ptr, Register::EAX)){
                    necessary = false;
                }
            }    

            if(necessary){
                writer.stream() << "movl " << arg(*return_->arg1) << ", %eax" << std::endl;
            }

            if(return_->arg2){
                spillsIfNecessary(Register::EBX, *return_->arg2);
                
                necessary = true;
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*return_->arg2)){
                    if(registers.inRegister(*ptr, Register::EBX)){
                        necessary = false;
                    }
                }    

                if(necessary){
                    writer.stream() << "movl " << arg(*return_->arg2) << ", %ebx" << std::endl;
                }
            }
        }
        
        if(function->context->size() > 0){
            writer.stream() << "addl $" << function->context->size() << " , %esp" << std::endl;
        }

        //The basic block must be ended before the jump
        endBasicBlock();

        writer.stream() << "leave" << std::endl;
        writer.stream() << "ret" << std::endl;
    }

    std::string toSubRegister(const std::string& reg){
        if(reg == "%eax"){
            return "%ah";
        } else if(reg == "%ebx"){
            return "%bh";
        } else if(reg == "%ecx"){
            return "%ch";
        } else if(reg == "%edx"){
            return "%dh";
        } else if(reg == "%edi"){
            return "%di";
        } else if(reg == "%esi"){
            return "%si";
        } else {
            assert(false);
        }
    }

    void setIfCc(const std::string& set, std::shared_ptr<tac::Quadruple>& quadruple){
        //We use EAX in order to avoid esi and edi that have not 8 byte version
        spills(Register::EAX);

        Register reg = Register::EAX;

        registers.setLocation(quadruple->result, reg);
        
        writer.stream() << "xorl " << regToString(reg) << ", " << regToString(reg) << std::endl;

        //The first argument is not important, it can be immediate, but the second must be a register
        if(auto* ptr = boost::get<int>(&quadruple->arg1)){
            auto reg = getReg();

            writer.stream() << "movl $" << *ptr << ", " << regToString(reg) << std::endl;

            writer.stream() << "cmpl " << arg(*quadruple->arg2) << ", " << regToString(reg) << std::endl;

            registers.release(reg);
        } else {
            writer.stream() << "cmpl " << arg(*quadruple->arg2) << ", " << arg(quadruple->arg1) << std::endl;
        }

        writer.stream() << set << " " << toSubRegister(regToString(reg)) << std::endl;
    }
    
    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        current = quadruple;
        
        if(!quadruple->op){
            //The fastest way to set a register to 0 is to use xorl
            if(tac::equals<int>(quadruple->arg1, 0)){
                Register reg = getRegNoMove(quadruple->result);
                writer.stream() << "xorl " << regToString(reg) << ", " << regToString(reg) << std::endl;            
            } 
            //In all the others cases, just move the value to the register
            else {
                Register reg = getRegNoMove(quadruple->result);
                writer.stream() << "movl " << arg(quadruple->arg1) << ", " << regToString(reg) << std::endl;            
            }
        } else {
            switch(*quadruple->op){
                case tac::Operator::ADD:
                {
                    auto result = quadruple->result;

                    //Optimize the special form a = a + b by using only one instruction
                    if(tac::equals<std::shared_ptr<Variable>>(quadruple->arg1, result)){
                        Register reg = getReg(quadruple->result);
                        
                        //a = a + 1 => increment a
                        if(tac::equals<int>(*quadruple->arg2, 1)){
                            writer.stream() << "incl " << regToString(reg) << std::endl;
                        }
                        //a = a + -1 => decrement a
                        else if(tac::equals<int>(*quadruple->arg2, -1)){
                            writer.stream() << "decl " << regToString(reg) << std::endl;
                        }
                        //In the other cases, perform a simple addition
                        else {
                            writer.stream() << "addl " << arg(*quadruple->arg2) << ", " << regToString(reg) << std::endl;
                        }
                    } 
                    //Optimize the special form a = b + a by using only one instruction
                    else if(tac::equals<std::shared_ptr<Variable>>(*quadruple->arg2, result)){
                        Register reg = getReg(quadruple->result);
                        
                        //a = 1 + a => increment a
                        if(tac::equals<int>(quadruple->arg1, 1)){
                            writer.stream() << "incl " << regToString(reg) << std::endl;
                        }
                        //a = -1 + a => decrement a
                        else if(tac::equals<int>(quadruple->arg1, -1)){
                            writer.stream() << "decl " << regToString(reg) << std::endl;
                        }
                        //In the other cases, perform a simple addition
                        else {
                            writer.stream() << "addl " << arg(quadruple->arg1) << ", " << regToString(reg) << std::endl;
                        }
                    } 
                    //In the other cases, move the first arg into the result register and then add the second arg into it
                    else {
                        Register reg = getRegNoMove(quadruple->result);
                        writer.stream() << "movl " << arg(quadruple->arg1) << ", " << regToString(reg) << std::endl;
                        writer.stream() << "addl " << arg(*quadruple->arg2) << ", " << regToString(reg) << std::endl;
                    }

                    break;
                }
                case tac::Operator::SUB:
                {
                    auto result = quadruple->result;
                    
                    //Optimize the special form a = a - b by using only one instruction
                    if(tac::equals<std::shared_ptr<Variable>>(quadruple->arg1, result)){
                        Register reg = getReg(quadruple->result);
                        
                        //a = a - 1 => decrement a
                        if(tac::equals<int>(*quadruple->arg2, 1)){
                            writer.stream() << "decl " << regToString(reg) << std::endl;
                        }
                        //a = a - -1 => increment a
                        else if(tac::equals<int>(*quadruple->arg2, -1)){
                            writer.stream() << "incl " << regToString(reg) << std::endl;
                        }
                        //In the other cases, perform a simple subtraction
                        else {
                            writer.stream() << "subl " << arg(*quadruple->arg2) << ", " << regToString(reg) << std::endl;
                        }
                    } 
                    //In the other cases, move the first arg into the result register and then subtract the second arg into it
                    else {
                        Register reg = getRegNoMove(quadruple->result);
                        writer.stream() << "movl " << arg(quadruple->arg1) << ", " << regToString(reg) << std::endl;
                        writer.stream() << "subl " << arg(*quadruple->arg2) << ", " << regToString(reg) << std::endl;
                    }
                    
                    break;
                }
                //TODO Simplify this generation
                case tac::Operator::MUL:
                {
                    bool fast = false;

                    //Form x = -1 * x
                    if(tac::equals<int>(quadruple->arg1, -1) && tac::equals<std::shared_ptr<Variable>>(*quadruple->arg2, quadruple->result)){
                        writer.stream() << "neg " << arg(quadruple->result) << std::endl;

                        fast = true;
                    } 
                    //Form x = x * -1
                    else if(tac::equals<int>(*quadruple->arg2, -1) && tac::equals<std::shared_ptr<Variable>>(quadruple->arg1, quadruple->result)){
                        writer.stream() << "neg " << arg(quadruple->result) << std::endl;

                        fast = true;
                    }

                    //If arg 1 is in eax
                    if(!fast){
                        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&quadruple->arg1)){
                            if(registers.inRegister(*ptr, Register::EAX)){
                                if((*ptr)->position().isTemporary() && !isNextLive(*ptr)){
                                    //If the arg is a variable, it will be matched to a register automatically
                                    if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2))
                                    {
                                        writer.stream() << "mull " << arg(*quadruple->arg2) << std::endl;
                                    } //If it's an immediate value, we have to move it in a register
                                    else if (boost::get<int>(&*quadruple->arg2)){
                                        auto reg = getReg();

                                        move(*quadruple->arg2, reg);
                                        writer.stream() << "mull " << regToString(reg) << std::endl;

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
                                    if(boost::get<std::shared_ptr<Variable>>(&quadruple->arg1))
                                    {
                                        writer.stream() << "mull " << arg(quadruple->arg1) << std::endl;
                                    } //If it's an immediate value, we have to move it in a register
                                    else if (boost::get<int>(&quadruple->arg1)){
                                        auto reg = getReg();

                                        move(quadruple->arg1, reg);
                                        writer.stream() << "mull " << regToString(reg) << std::endl;

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
                        copy(quadruple->arg1, Register::EAX);
                        
                        //If the arg is a variable, it will be matched to a register automatically
                        if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2))
                        {
                            writer.stream() << "mull " << arg(*quadruple->arg2) << std::endl;
                        } //If it's an immediate value, we have to move it in a register
                        else if (boost::get<int>(&*quadruple->arg2)){
                            auto reg = getReg();

                            move(*quadruple->arg2, reg);
                            writer.stream() << "mull " << regToString(reg) << std::endl;

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
                    
                    copy(quadruple->arg1, Register::EAX);
                    writer.stream() << "xorl %edx, %edx" << std::endl;

                    //If the second arg is immediate, we have to move it in a register
                    if(boost::get<int>(&*quadruple->arg2)){
                        auto reg = getReg();

                        move(*quadruple->arg2, reg);
                        writer.stream() << "divl " << regToString(reg) << std::endl;

                        if(registers.reserved(reg)){
                            registers.release(reg);
                        }
                    } else {
                        writer.stream() << "divl " << arg(*quadruple->arg2) << std::endl;
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
                    
                    copy(quadruple->arg1, Register::EAX);
                    writer.stream() << "xorl %edx, %edx" << std::endl;

                    //If the second arg is immediate, we have to move it in a register
                    if(boost::get<int>(&*quadruple->arg2)){
                        auto reg = getReg();

                        move(*quadruple->arg2, reg);
                        writer.stream() << "divl " << regToString(reg) << std::endl;

                        if(registers.reserved(reg)){
                            registers.release(reg);
                        }
                    } else {
                        writer.stream() << "divl " << arg(*quadruple->arg2) << std::endl;
                    }

                    //result is in edx (no need to move it now)
                    registers.setLocation(quadruple->result, Register::EDX);

                    registers.release(Register::EAX);

                    break;            
                case tac::Operator::MINUS:
                {
                    //If arg is immediate, we have to move it in a register
                    if(boost::get<int>(&quadruple->arg1)){
                        auto reg = getReg();

                        move(quadruple->arg1, reg);
                        writer.stream() << "neg " << regToString(reg) << std::endl;

                        if(registers.reserved(reg)){
                            registers.release(reg);
                        }
                    } else {
                        writer.stream() << "neg " << arg(quadruple->arg1) << std::endl;
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
                   assert(boost::get<std::shared_ptr<Variable>>(&quadruple->arg1));
                   assert(boost::get<int>(&*quadruple->arg2));

                   int offset = boost::get<int>(*quadruple->arg2);
                   auto variable = boost::get<std::shared_ptr<Variable>>(quadruple->arg1);
   
                   Register reg = getRegNoMove(quadruple->result);
                   writer.stream() << "movl " << toString(variable, offset) << ", " << regToString(reg) << std::endl;

                   break;
                }
                case tac::Operator::DOT_ASSIGN:
                {
                    assert(boost::get<int>(&quadruple->arg1));

                    int offset = boost::get<int>(quadruple->arg1);

                    writer.stream() << "movl " << arg(*quadruple->arg2) << ", " << toString(quadruple->result, offset) << std::endl;

                    break;
                }
                case tac::Operator::ARRAY:
                {
                    assert(boost::get<std::shared_ptr<Variable>>(&quadruple->arg1));

                    Register reg = getRegNoMove(quadruple->result);

                    writer.stream() << "movl " << toString(boost::get<std::shared_ptr<Variable>>(quadruple->arg1), *quadruple->arg2) << ", " << regToString(reg) << std::endl;
                    
                    break;            
                }
                case tac::Operator::ARRAY_ASSIGN:
                    writer.stream() << "movl " << arg(*quadruple->arg2) << ", " << toString(quadruple->result, quadruple->arg1) << std::endl;

                    break;
                case tac::Operator::PARAM:
                {
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&quadruple->arg1)){
                        if((*ptr)->type().isArray()){
                            auto position = (*ptr)->position();

                            if(position.isGlobal()){
                                Register reg = getReg();
                                
                                auto offset = size((*ptr)->type().base()) * (*ptr)->type().size();

                                writer.stream() << "movl $V" << position.name() << ", " << regToString(reg) << std::endl;
                                writer.stream() << "addl $" << offset << ", " << regToString(reg) << std::endl;
                                writer.stream() << "pushl " << regToString(reg) << std::endl;

                                registers.release(reg);
                            } else if(position.isStack()){
                                Register reg = getReg();

                                writer.stream() << "movl %ebp, " << regToString(reg) << std::endl;
                                writer.stream() << "addl $" << (-position.offset()) << ", " << regToString(reg) << std::endl;
                                writer.stream() << "pushl " << regToString(reg) << std::endl;
                                
                                registers.release(reg);
                            } else if(position.isParameter()){
                                writer.stream() << "pushl " << position.offset() << "(%ebp)" << std::endl;
                            }
                        } else {
                            writer.stream() << "pushl " << arg(quadruple->arg1) << std::endl;
                        }
                    } else {
                        writer.stream() << "pushl " << arg(quadruple->arg1) << std::endl;
                    }

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

            writer.stream() << "movl $" << *ptr << ", " << regToString(reg) << std::endl;

            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "cmpl " << arg(*if_->arg2) << ", " << regToString(reg) << std::endl;

            registers.release(reg);
        } else {
            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "cmpl " << arg(*if_->arg2) << ", " << arg(if_->arg1) << std::endl;
        }
    }

    template<typename T>
    void compareUnary(T& if_){
        if(auto* ptr = boost::get<int>(&if_->arg1)){
            auto reg = getReg();

            writer.stream() << "movl $" << *ptr << ", " << regToString(reg) << std::endl;

            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "orl " << regToString(reg) << ", " << regToString(reg) << std::endl;

            registers.release(reg);
        } else {
            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "orl " << arg(if_->arg1) << ", " << arg(if_->arg1) << std::endl;
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
    
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;

    auto size = function->context->size();
    //Only if necessary, allocates size on the stack for the local variables
    if(size > 0){
        writer.stream() << "subl $" << size << " , %esp" << std::endl;
    }
    
    auto iter = function->context->begin();
    auto end = function->context->end();

    for(; iter != end; iter++){
        auto var = iter->second;
        if(var->type().isArray() && var->position().isStack()){
            int position = -var->position().offset();

            writer.stream() << "movl $" << var->type().size() << ", " << position << "(%ebp)" << std::endl;

            if(var->type().base() == BaseType::INT){
                for(unsigned int i = 0; i < var->type().size(); ++i){
                    writer.stream() << "movl $0, " << (position -= 4) << "(%ebp)" << std::endl;
                }
            } else if(var->type().base() == BaseType::STRING){
                for(unsigned int i = 0; i < var->type().size(); ++i){
                    writer.stream() << "movl $0, " << (position -= 4) << "(%ebp)" << std::endl;
                    writer.stream() << "movl $0, " << (position -= 4) << "(%ebp)" << std::endl;
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
        writer.stream() << "addl $" << size << " , %esp" << std::endl;
    }
    
    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void as::IntelX86CodeGenerator::writeRuntimeSupport(){
    writer.stream() << ".text" << std::endl
                    << ".globl _start" << std::endl
                    
                    << "_start:" << std::endl
                    << "call main" << std::endl
                    << "movl $1, %eax" << std::endl
                    << "xorl %ebx, %ebx" << std::endl
                    << "int $0x80" << std::endl;
}

void addPrintIntegerFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "print_integer:" << std::endl
             << "pushl %ebp" << std::endl
             << "movl %esp, %ebp" << std::endl

            //Save registers
            << "pushl %eax" << std::endl
            << "pushl %ebx" << std::endl
            << "pushl %ecx" << std::endl
            << "pushl %edx" << std::endl

             << "movl 8(%ebp), %eax" << std::endl
             << "xorl %esi, %esi" << std::endl

             //If the number is negative, we print the - and then the number
             << "cmpl $0, %eax" << std::endl
             << "jge loop" << std::endl

             << "neg %eax" << std::endl
             << "pushl %eax" << std::endl //We push eax to not loose it from print_string
            
             //Print "-" 
             << "pushl $S2" << std::endl
             << "pushl $1" << std::endl
             << "call print_string" << std::endl
             << "addl $8, %esp" << std::endl

             //Get the the valueof eax again
             << "popl %eax" << std::endl

             << "loop:" << std::endl
             << "movl $0, %edx" << std::endl
             << "movl $10, %ebx" << std::endl
             << "divl %ebx" << std::endl
             << "addl $48, %edx" << std::endl
             << "pushl %edx" << std::endl
             << "incl %esi" << std::endl
             << "cmpl $0, %eax" << std::endl
             << "jz   next" << std::endl
             << "jmp loop" << std::endl

             << "next:" << std::endl
             << "cmpl $0, %esi" << std::endl
             << "jz   exit" << std::endl
             << "decl %esi" << std::endl

             << "movl $4, %eax" << std::endl
             << "movl %esp, %ecx" << std::endl
             << "movl $1, %ebx" << std::endl
             << "movl $1, %edx" << std::endl
             << "int  $0x80" << std::endl

             << "addl $4, %esp" << std::endl

             << "jmp  next" << std::endl

             << "exit:" << std::endl

            //Restore registers
            << "popl %edx" << std::endl
            << "popl %ecx" << std::endl
            << "popl %ebx" << std::endl
            << "popl %eax" << std::endl

             << "leave" << std::endl
             << "ret" << std::endl;
}

void addPrintLineFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "print_line:" << std::endl;
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;

    writer.stream() << "pushl $S1" << std::endl;
    writer.stream() << "pushl $1" << std::endl;
    writer.stream() << "call print_string" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void addPrintStringFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "print_string:" << std::endl;
    writer.stream() << "pushl %ebp" << std::endl;
    writer.stream() << "movl %esp, %ebp" << std::endl;
    
    //Save registers
    writer.stream() << "pushl %eax" << std::endl;
    writer.stream() << "pushl %ebx" << std::endl;
    writer.stream() << "pushl %ecx" << std::endl;
    writer.stream() << "pushl %edx" << std::endl;

    writer.stream() << "movl $0, %esi" << std::endl;

    writer.stream() << "movl $4, %eax" << std::endl;
    writer.stream() << "movl $1, %ebx" << std::endl;
    writer.stream() << "movl 12(%ebp), %ecx" << std::endl;
    writer.stream() << "movl 8(%ebp), %edx" << std::endl;
    writer.stream() << "int $0x80" << std::endl;

    //Restore registers
    writer.stream() << "popl %edx" << std::endl;
    writer.stream() << "popl %ecx" << std::endl;
    writer.stream() << "popl %ebx" << std::endl;
    writer.stream() << "popl %eax" << std::endl;

    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}

void addConcatFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "concat:" << std::endl
        << "pushl %ebp" << std::endl
        << "movl %esp, %ebp" << std::endl

        << "movl 16(%ebp), %edx" << std::endl
        << "movl 8(%ebp), %ecx" << std::endl
        << "addl %ecx, %edx" << std::endl

        << "pushl %edx" << std::endl
        << "call eddi_alloc" << std::endl
        << "addl $4, %esp" << std::endl

        << "movl %eax, -4(%ebp)" << std::endl
        << "movl %eax, %ecx" << std::endl
        << "movl $0, %eax" << std::endl

        << "movl 16(%ebp), %ebx" << std::endl
        << "movl 20(%ebp), %edx" << std::endl

        << "copy_concat_1:" << std::endl
        << "cmpl $0, %ebx" << std::endl
        << "je end_concat_1"  << std::endl
        << "movb (%edx), %al" << std::endl
        << "movb %al, (%ecx)" << std::endl
        << "addl $1, %ecx" << std::endl
        << "addl $1, %edx" << std::endl
        << "subl $1, %ebx" << std::endl
        << "jmp copy_concat_1" << std::endl
        << "end_concat_1" << ":" << std::endl

        << "movl 8(%ebp), %ebx" << std::endl
        << "movl 12(%ebp), %edx" << std::endl

        << "copy_concat_2:" << std::endl
        << "cmpl $0, %ebx" << std::endl
        << "je end_concat_2"  << std::endl
        << "movb (%edx), %al" << std::endl
        << "movb %al, (%ecx)" << std::endl
        << "addl $1, %ecx" << std::endl
        << "addl $1, %edx" << std::endl
        << "subl $1, %ebx" << std::endl
        << "jmp copy_concat_2" << std::endl
        << "end_concat_2:" << std::endl

        << "movl 16(%ebp), %ebx" << std::endl
        << "movl 8(%ebp), %ecx" << std::endl
        << "addl %ecx, %ebx" << std::endl

        << "movl -4(%ebp), %eax" << std::endl

        << "leave" << std::endl
        << "ret" << std::endl;
}

void addAllocFunction(AssemblyFileWriter& writer){
    writer.stream() << std::endl;
    writer.stream() << "eddi_alloc:" << std::endl
        << "pushl %ebp" << std::endl
        << "movl %esp, %ebp" << std::endl

        //Save registers
        << "pushl %ebx" << std::endl
        << "pushl %ecx" << std::endl
        << "pushl %edx" << std::endl

        << "movl 8(%ebp), %ecx" << std::endl
        << "movl Veddi_remaining, %ebx" << std::endl

        << "cmpl %ebx, %ecx" << std::endl
        << "jle alloc_normal" << std::endl

        //Get the current address
        << "movl $45, %eax" << std::endl        //45 = sys_brk
        << "xorl %ebx, %ebx" << std::endl       //get end
        << "int  $0x80" << std::endl
       
        //%eax is the current address 
        << "movl %eax, %esi" << std::endl

        //Alloc new block of 16384K from the current address
        << "movl %eax, %ebx" << std::endl
        << "addl $16384, %ebx" << std::endl
        << "movl $45, %eax" << std::endl        //45 = sys_brk
        << "int  $0x80" << std::endl

        //zero'd the new block
        << "movl %eax, %edi" << std::endl       //edi = start of block

        << "subl $4, %edi" << std::endl         //EDI points to the last DWORD available to us
        << "movl $4096, %ecx" << std::endl         //this many DWORDs were allocated
        << "xorl %eax, %eax"  << std::endl         //   ; will write with zeroes
        << "std"  << std::endl         //        ; walk backwards
        << "rep stosl"  << std::endl         //      ; write all over the reserved area
        << "cld"  << std::endl         //        ; bring back the DF flag to normal state

        << "movl %esi, %eax" << std::endl

        //We now have 16K of available memory starting at %esi
        << "movl $16384, Veddi_remaining" << std::endl
        << "movl %esi, Veddi_current" << std::endl

        << "alloc_normal:" << std::endl

        //old = current
        << "movl Veddi_current, %eax" << std::endl
        
        //current += size
        << "movl Veddi_current, %ebx" << std::endl
        << "addl %ecx, %ebx" << std::endl
        << "movl %ebx, Veddi_current" << std::endl
        
        //remaining -= size
        << "movl Veddi_remaining, %ebx" << std::endl
        << "subl %ecx, %ebx" << std::endl
        << "movl %ebx, Veddi_remaining" << std::endl
       
        << "alloc_end:" << std::endl

        //Restore registers
        << "popl %edx" << std::endl
        << "popl %ecx" << std::endl
        << "popl %ebx" << std::endl

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
    writer.stream() << std::endl << ".data" << std::endl;
     
    for(auto it : context->getVariables()){
        Type type = it.second->type();
        
        //The const variables are not stored
        if(type.isConst()){
            continue;
        }

        if(type.isArray()){
            writer.stream() << "V" << it.second->position().name() << ":" <<std::endl;
            writer.stream() << ".rept " << type.size() << std::endl;

            if(type.base() == BaseType::INT){
                writer.stream() << ".long 0" << std::endl;
            } else if(type.base() == BaseType::STRING){
                writer.stream() << ".long S3" << std::endl;
                writer.stream() << ".long 0" << std::endl;
            }

            writer.stream() << ".endr" << std::endl;
            writer.stream() << ".long " << type.size() << std::endl;
        } else {
            if (type.base() == BaseType::INT) {
                writer.stream() << ".size V" << it.second->position().name() << ", 4" << std::endl;
                writer.stream() << "V" << it.second->position().name() << ":" << std::endl;
                writer.stream() << ".long " << boost::get<int>(it.second->val()) << std::endl;
            } else if (type.base() == BaseType::STRING) {
                auto value = boost::get<std::pair<std::string, int>>(it.second->val());
  
                //If that's not the case, there is a problem with the pool 
                assert(value.first.size() > 0);
                
                writer.stream() << ".size V" << it.second->position().name() << ", 8" << std::endl;
                writer.stream() << "V" << it.second->position().name() << ":" << std::endl;
                writer.stream() << ".long " << pool.label(value.first) << std::endl;
                writer.stream() << ".long " << value.second << std::endl;
            }
        }
    }
    
    for (auto it : pool.getPool()){
        writer.stream() << it.second << ":" << std::endl;
        writer.stream() << ".string " << it.first << std::endl;
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
