//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTEL_STATEMENT_COMPILER_H
#define INTEL_STATEMENT_COMPILER_H

#include <memory>
#include <unordered_set>

#include "Utils.hpp"
#include "Registers.hpp"

#include "tac/Utils.hpp"

using namespace eddic;

namespace eddic {

namespace as {

template<typename Register, typename FloatRegister>
struct IntelStatementCompiler {
    //The current function being compiled
    std::shared_ptr<tac::Function> function;
    
    AssemblyFileWriter& writer;
    Registers<Register> registers;
    Registers<FloatRegister> float_registers;

    std::unordered_set<std::shared_ptr<tac::BasicBlock>> blockUsage;

    std::unordered_set<std::shared_ptr<Variable>> written;

    bool last;
    bool ended;

    tac::Statement current;
    tac::Statement next;
   
    IntelStatementCompiler(AssemblyFileWriter& w, std::vector<Register> r, std::vector<FloatRegister> fr, std::shared_ptr<tac::Function> f) : function(f), writer(w), 
            registers(r, std::make_shared<Variable>("__fake_int__", newSimpleType(BaseType::INT), Position(PositionType::TEMPORARY))),
            float_registers(fr, std::make_shared<Variable>("__fake_float__", newSimpleType(BaseType::FLOAT), Position(PositionType::TEMPORARY))) {
        last = ended = false;        
    } 
    
    virtual void div(std::shared_ptr<tac::Quadruple> quadruple) = 0;
    virtual void mod(std::shared_ptr<tac::Quadruple> quadruple) = 0;
    
    virtual std::string getMnemonicSize() = 0;
    virtual std::string getFloatPrefix() = 0;
    
    virtual std::string getFloatMove() = 0;
    virtual std::string getFloatAdd() = 0;
    virtual std::string getFloatSub() = 0;
    virtual std::string getFloatMul() = 0;
    virtual std::string getFloatDiv() = 0;
    virtual std::string getSizedMove() = 0;
    
    virtual Register getReturnRegister1() = 0;
    virtual Register getReturnRegister2() = 0;
    virtual Register getStackPointerRegister() = 0;
    virtual Register getBasePointerRegister() = 0;

    bool isFloatVar(std::shared_ptr<Variable> variable){
        return variable->type() == BaseType::FLOAT;
    }
    
    bool isIntVar(std::shared_ptr<Variable> variable){
        return variable->type() == BaseType::INT;
    }

    //TODO Verify these two functions
    void allocateStackSpace(unsigned int space){
        writer.stream() << "add " << getStackPointerRegister() << ", " << space << std::endl;
    }

    void deallocateStackSpace(unsigned int space){
        writer.stream() << "add " << getStackPointerRegister() << ", " << space << std::endl;
    }
    
    std::string toString(std::shared_ptr<Variable> variable, int offset){
        auto position = variable->position();

        assert(!position.isTemporary());

        if(position.isStack()){
            return "[" + regToString(getBasePointerRegister()) + " + " + ::toString(-position.offset() + offset) + "]";
        } else if(position.isParameter()){
            //The case of array is special because only the address is passed, not the complete array
            if(variable->type().isArray())
            {
                Register reg = getReg();

                writer.stream() << "mov " << reg << ", [" + regToString(getBasePointerRegister()) + " + " << ::toString(position.offset()) << "]" << std::endl;

                registers.release(reg);

                return "[" + reg + "+" + ::toString(offset) + "]";
            } 
            //In the other cases, the value is passed, so we can compute the offset directly
            else {
                return "[" + regToString(getBasePointerRegister()) + " + " + ::toString(position.offset() + offset) + "]";
            }
        } else if(position.isGlobal()){
            return "[V" + position.name() + "+" + ::toString(offset) + "]";
        } 

        assert(false && "Should never get there");
    }
    
    std::string toString(std::shared_ptr<Variable> variable, tac::Argument offset){
        if(auto* ptr = boost::get<int>(&offset)){
            return toString(variable, *ptr);
        }
        
        assert(boost::get<std::shared_ptr<Variable>>(&offset));

        auto* offsetVariable = boost::get<std::shared_ptr<Variable>>(&offset);
        auto position = variable->position();

        auto offsetReg = getReg(*offsetVariable);
        
        assert(!position.isTemporary());
        
        if(position.isStack()){
            return "[" + regToString(getBasePointerRegister()) + " + " + ::toString(-1 * (position.offset())) + "]";//TODO Verify
        } else if(position.isParameter()){
            Register reg = getReg();
            
            writer.stream() << "mov " << reg << ", [" + regToString(getBasePointerRegister()) + " + " << ::toString(position.offset()) << "]" << std::endl;

            registers.release(reg);

            return "[" + reg + "+" + offsetReg + "]";
        } else if(position.isGlobal()){
            return "[" + offsetReg + "+V" + position.name() + "]";
        } 

        assert(false && "Should never get there");
    }
    
    void copy(tac::Argument argument, FloatRegister reg){
        assert(isVariable(argument) || isFloat(argument));

        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            auto variable = *ptr;

            //If the variable is hold in a register, just move the register value
            if(float_registers.inRegister(variable)){
                auto oldReg = float_registers[variable];
                
                writer.stream() << getFloatMove() << reg << ", " << oldReg << std::endl;
            } else {
                auto position = variable->position();
                
                //The temporary should have been handled by the preceding condition (hold in a register)
                assert(!position.isTemporary());

                if(position.isStack()){
                    writer.stream() << getFloatMove() << reg << ", [" + regToString(getBasePointerRegister()) + " + " << (-1 * position.offset()) << "]" << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << getFloatMove() << reg << ", [" + regToString(getBasePointerRegister()) + " + " << position.offset() << "]" << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << getFloatMove() << reg << ", [V" << position.name() << "]" << std::endl;
                } 
            }
        } else if(boost::get<double>(&argument)){
            Register gpreg = getReg();
            
            writer.stream() << "mov " << gpreg << ", " << arg(argument) << std::endl;
            writer.stream() << getSizedMove() << reg << ", " << gpreg << std::endl;

            registers.release(gpreg);
        }
    }
    
    void copy(tac::Argument argument, Register reg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            auto variable = *ptr;

            //If the variable is hold in a register, just move the register value
            if(registers.inRegister(variable)){
                auto oldReg = registers[variable];
                
                writer.stream() << "mov " << reg << ", " << oldReg << std::endl;
            } else {
                auto position = variable->position();
                
                //The temporary should have been handled by the preceding condition (hold in a register)
                assert(!position.isTemporary());

                if(position.isStack()){
                    writer.stream() << "mov " << reg << ", [" + regToString(getBasePointerRegister()) + " + " << (-1 * position.offset()) << "]" << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << "mov " << reg << ", [" + regToString(getBasePointerRegister()) + " + " << position.offset() << "]" << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << "mov " << reg << ", [V" << position.name() << "]" << std::endl;
                } 
            } 
        } else {
            //If it's a constant (int, double, string), just move it
            writer.stream() << "mov " << reg << ", " << arg(argument) << std::endl;
        }
    }
    
    void move(tac::Argument argument, FloatRegister reg){
        assert(isVariable(argument) || isFloat(argument));

        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            auto variable = *ptr;

            //If the variable is hold in a register, just move the register value
            if(float_registers.inRegister(variable)){
                auto oldReg = float_registers[variable];
               
                //Only if the variable is not already on the same register 
                if(oldReg != reg){
                    writer.stream() << getFloatMove() << reg << ", " << oldReg << std::endl;

                    //There is nothing more in the old register
                    float_registers.remove(variable);
                }
            } else {
                auto position = variable->position();

                //The temporary should have been handled by the preceding condition (hold in a register)
                assert(!position.isTemporary());

                if(position.isStack()){
                    writer.stream() << getFloatMove() << reg << ", [" + regToString(getBasePointerRegister()) + " + " << (-1 * position.offset()) << "]" << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << getFloatMove() << reg << ", [" + regToString(getBasePointerRegister()) + " + " << position.offset() << "]" << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << getFloatMove() << reg << ", [V" << position.name() << "]" << std::endl;
                } 
            }
            
            //The variable is now held in the new register
            float_registers.setLocation(variable, reg);
        } else if(boost::get<double>(&argument)){
            Register gpreg = getReg();
            
            writer.stream() << "mov " << gpreg << ", " << arg(argument) << std::endl;
            writer.stream() << getSizedMove() << reg << ", " << gpreg << std::endl;

            registers.release(gpreg);
        }
    }
    
    void move(tac::Argument argument, Register reg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
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

                //The temporary should have been handled by the preceding condition (hold in a register)
                assert(!position.isTemporary());

                if(position.isStack()){
                    writer.stream() << "mov " << reg << ", [" + regToString(getBasePointerRegister()) + " + " << (-1 * position.offset()) << "]" << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << "mov " << reg << ", [" + regToString(getBasePointerRegister()) + " + " << position.offset() << "]" << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << "mov " << reg << ", [V" << position.name() << "]" << std::endl;
                }
            } 
            
            //The variable is now held in the new register
            registers.setLocation(variable, reg);
        } else {
            //If it's a constant (int, double, string), just move it
            writer.stream() << "mov " << reg << ", " << arg(argument) << std::endl;
        }
    }
    
    void spills(FloatRegister reg){
        //If the register is not used, there is nothing to spills
        if(float_registers.used(reg)){
            auto variable = float_registers[reg];

            //If the variable has not been written, there is no need to spill it
            if(written.find(variable) != written.end()){
                auto position = variable->position();
                if(position.isStack()){
                    writer.stream() << getFloatMove() << "[" + regToString(getBasePointerRegister()) + " + " << (-1 * position.offset()) << "], " << reg << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << getFloatMove() << "[" + regToString(getBasePointerRegister()) + " + " << position.offset() << "], " << reg << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << getFloatMove() << "[V" << position.name() << "], " << reg << std::endl;
                } else if(position.isTemporary()){
                    //If the variable is live, move it to another register, else do nothing
                    if(isLive(variable)){
                        float_registers.remove(variable);
                        float_registers.reserve(reg);

                        auto newReg = getFloatRegNoMove(variable);
                        writer.stream() << getFloatMove() << newReg << ", " << reg << std::endl;

                        float_registers.release(reg);

                        return; //Return here to avoid erasing variable from variables
                    }
                }
            }
            
            //The variable is no more contained in the register
            float_registers.remove(variable);

            //The variable has not been written now
            written.erase(variable);
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
                    writer.stream() << "mov [" + regToString(getBasePointerRegister()) + " + " << (-1 * position.offset()) << "], " << reg << std::endl; 
                } else if(position.isParameter()){
                    writer.stream() << "mov [" + regToString(getBasePointerRegister()) + " + " << position.offset() << "], " << reg << std::endl; 
                } else if(position.isGlobal()){
                    writer.stream() << "mov [V" << position.name() << "], " << reg << std::endl;
                } else if(position.isTemporary()){
                    //If the variable is live, move it to another register, else do nothing
                    if(isLive(variable)){
                        registers.remove(variable);
                        registers.reserve(reg);

                        Register newReg = getRegNoMove(variable);
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

    void setIfCc(const std::string& set, std::shared_ptr<tac::Quadruple>& quadruple){
        Register reg = getRegNoMove(quadruple->result);

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

        written.insert(quadruple->result);
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
   
    void setLast(bool l){
        last = l;
    }

    bool isLive(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, std::shared_ptr<Variable> variable){
        if(liveness.find(variable) != liveness.end()){
            return liveness[variable];   
        } else {
            return !variable->position().isTemporary();
        }
    }

    bool isLive(std::shared_ptr<Variable> variable, tac::Statement statement){
        assert(tac::is<std::shared_ptr<tac::Quadruple>>(statement) || tac::is<std::shared_ptr<tac::IfFalse>>(statement));

        if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&statement)){
            return isLive((*ptr)->liveness, variable);
        } else if (auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&statement)){
            return isLive((*ptr)->liveness, variable);
        } 

        return false;
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

    template<typename Reg>
    void safeMove(Registers<Reg>& registers, std::shared_ptr<Variable> variable, Reg reg){
        if(registers.used(reg)){
            if(registers[reg] != variable){
                spills(reg);

                move(variable, reg);
            }
        } else {
            move(variable, reg);
        }
    }

    template<typename Reg>
    Reg getFreeReg(Registers<Reg>& registers){
        //Try to get a free register 
        for(Reg reg : registers){
            if(!registers.used(reg)){
                return reg;
            } else if(!registers.reserved(reg) && !isLive(registers[reg])){
                registers.remove(registers[reg]);

                return reg;
            }
        }
       
        //There are no free register, take one
        Reg reg = registers.first();
        bool found = false;

        //First, try to take a register that doesn't need to be spilled (variable has not modified)
        for(Reg remaining : registers){
            if(!registers.reserved(remaining)){
                if(written.find(registers[remaining]) == written.end()){
                    reg = remaining;
                    found = true;
                }
            }
        }
       
        //If there is no registers that doesn't need to be spilled, take the first one not reserved 
        if(!found){
            for(Reg remaining : registers){
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
   
    template<typename Reg> 
    Reg getReg(Registers<Reg>& registers, std::shared_ptr<Variable> variable, bool doMove){
        //The variable is already in a register
        if(registers.inRegister(variable)){
            return registers[variable];
        }
       
        Reg reg = getFreeReg(registers);

        if(doMove){
            move(variable, reg);
        }

        registers.setLocation(variable, reg);

        return reg;
    }
    
    template<typename Reg>
    Reg getReg(Registers<Reg>& registers){
        Reg reg = getFreeReg(registers);

        registers.reserve(reg);

        return reg;
    }
    
    void safeMove(std::shared_ptr<Variable> variable, Register reg){
        return safeMove(registers, variable, reg);
    }

    void safeMove(std::shared_ptr<Variable> variable, FloatRegister reg){
        return safeMove(registers, variable, reg);
    }
    
    Register getFreeReg(){
        return getFreeReg(registers); 
    }
    
    FloatRegister getFreeFloatReg(){
        return getFreeReg(float_registers); 
    }
    
    FloatRegister getFloatRegNoMove(std::shared_ptr<Variable> variable){
        return getReg(float_registers, variable, false);
    }

    FloatRegister getFloatReg(std::shared_ptr<Variable> variable){
        return getReg(float_registers, variable, true);
    }
    
    Register getRegNoMove(std::shared_ptr<Variable> variable){
        return getReg(registers, variable, false);
    }

    Register getReg(std::shared_ptr<Variable> variable){
        return getReg(registers, variable, true);
    }

    Register getReg(){
        return getReg(registers);
    }
    
    FloatRegister getFloatReg(){
        return getReg(float_registers);
    }

    inline std::string toFloatString(double arg){
        std::string str = ::toString(arg);

        if(str.find(".") == std::string::npos){
            return str + ".0";
        }

        return str;
    }

    std::string arg(tac::Argument argument){
        if(auto* ptr = boost::get<int>(&argument)){
            return ::toString(*ptr);
        } else if(auto* ptr = boost::get<double>(&argument)){
            return getFloatPrefix() + "(" + toFloatString(*ptr) + ")";
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            return *ptr;
        } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            if(isFloatVar(*ptr)){
                if((*ptr)->position().isTemporary()){
                    return regToString(getFloatRegNoMove(*ptr));
                } else {
                    return regToString(getFloatReg(*ptr));
                }
            } else {
                if((*ptr)->position().isTemporary()){
                    return regToString(getRegNoMove(*ptr));
                } else {
                    return regToString(getReg(*ptr));
                }
            }
        }

        assert(false && "Should never get there");
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

    template<typename Reg>
    void spillsAll(Registers<Reg> registers){
        for(auto reg : registers){
            //The register can be reserved if the ending occurs in a special break case
            if(!registers.reserved(reg) && registers.used(reg)){
                auto variable = registers[reg];

                if(!variable->position().isTemporary()){
                    spills(reg);    
                }
            }
        }
    }
    
    void endBasicBlock(){
        spillsAll(registers);
        spillsAll(float_registers);

        ended = true;
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
    void compareFloatBinary(T& if_){
        //Comparisons of constant should have been handled by the optimizer
        assert(!(isFloat(if_->arg1) && isFloat(*if_->arg2))); 

        //If both args are variables
        if(isVariable(if_->arg1) && isVariable(*if_->arg2)){
            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "ucomisd " << arg(if_->arg1) << ", " << arg(*if_->arg2) << std::endl;
        } else if(isVariable(if_->arg1) && isFloat(*if_->arg2)){
            auto reg = getFloatReg();

            copy(*if_->arg2, reg);

            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "ucomisd " << arg(if_->arg1) << ", " << reg << std::endl;
            
            float_registers.release(reg);
        } else if(isFloat(if_->arg1) && isVariable(*if_->arg2)){
            auto reg = getFloatReg();

            copy(if_->arg1, reg);

            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "ucomisd " << reg << ", " << arg(*if_->arg2) << std::endl;
            
            float_registers.release(reg);
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

    bool isFloatOperator(tac::BinaryOperator op){
        return op >= tac::BinaryOperator::FE && op <= tac::BinaryOperator::FL;
    }

    void compile(std::shared_ptr<tac::IfFalse> ifFalse){
        current = ifFalse;

        if(ifFalse->op){
            //Depending on the type of the operator, do a float or a int comparison
            if(isFloatOperator(*ifFalse->op)){
                compareFloatBinary(ifFalse);
                
                switch(*ifFalse->op){
                    case tac::BinaryOperator::FE:
                        writer.stream() << "jne " << ifFalse->block->label << std::endl;
                        writer.stream() << "jp " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FNE:
                        writer.stream() << "je " << ifFalse->block->label << std::endl;
                        writer.stream() << "jp " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FL:
                        writer.stream() << "jae " << ifFalse->block->label << std::endl;
                        writer.stream() << "jp " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FLE:
                        writer.stream() << "ja " << ifFalse->block->label << std::endl;
                        writer.stream() << "jp " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FG:
                        writer.stream() << "jbe " << ifFalse->block->label << std::endl;
                        writer.stream() << "jp " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FGE:
                        writer.stream() << "jb " << ifFalse->block->label << std::endl;
                        writer.stream() << "jp " << ifFalse->block->label << std::endl;
                        break;
                    default:
                        assert(false && "This operation is not a float operator");
                        break;
                }
            } else {
                compareBinary(ifFalse);
            
                switch(*ifFalse->op){
                    case tac::BinaryOperator::EQUALS:
                        writer.stream() << "jne " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::NOT_EQUALS:
                        writer.stream() << "je " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::LESS:
                        writer.stream() << "jge " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::LESS_EQUALS:
                        writer.stream() << "jg " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::GREATER:
                        writer.stream() << "jle " << ifFalse->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::GREATER_EQUALS:
                        writer.stream() << "jl " << ifFalse->block->label << std::endl;
                        break;
                    default:
                        assert(false && "This operation is not a float operator");
                        break;
                }
            }
        } else {
            compareUnary(ifFalse);

            writer.stream() << "jz " << ifFalse->block->label << std::endl;
        }
    }

    void compile(std::shared_ptr<tac::If> if_){
        current = if_;

        if(if_->op){
            //Depending on the type of the operator, do a float or a int comparison
            if(isFloatOperator(*if_->op)){
                compareFloatBinary(if_);
            
                switch(*if_->op){
                    case tac::BinaryOperator::FE:
                        writer.stream() << "je " << if_->block->label << std::endl;
                        writer.stream() << "jp " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FNE:
                        writer.stream() << "jne " << if_->block->label << std::endl;
                        writer.stream() << "jp " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FL:
                        writer.stream() << "jb " << if_->block->label << std::endl;
                        writer.stream() << "jp " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FLE:
                        writer.stream() << "jbe " << if_->block->label << std::endl;
                        writer.stream() << "jp " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FG:
                        writer.stream() << "ja " << if_->block->label << std::endl;
                        writer.stream() << "jp " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::FGE:
                        writer.stream() << "jae " << if_->block->label << std::endl;
                        writer.stream() << "jp " << if_->block->label << std::endl;
                        break;
                    default:
                        assert(false && "This operation is not a float operator");
                        break;
                }
            } else {
                compareBinary(if_);
            
                switch(*if_->op){
                    case tac::BinaryOperator::EQUALS:
                        writer.stream() << "je " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::NOT_EQUALS:
                        writer.stream() << "jne " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::LESS:
                        writer.stream() << "jl " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::LESS_EQUALS:
                        writer.stream() << "jle " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::GREATER:
                        writer.stream() << "jg " << if_->block->label << std::endl;
                        break;
                    case tac::BinaryOperator::GREATER_EQUALS:
                        writer.stream() << "jge " << if_->block->label << std::endl;
                        break;
                    default:
                        assert(false && "This operation is not a float operator");
                        break;
                }
            }

        } else {
            compareUnary(if_);

            writer.stream() << "jnz " << if_->block->label << std::endl;
        }
    }
    
    void compile(std::shared_ptr<tac::Goto> goto_){
        current = goto_;

        //The basic block must be ended before the jump
        endBasicBlock();

        writer.stream() << "jmp " << goto_->block->label << std::endl; 
    }

    void compile(std::shared_ptr<tac::Call> call){
        current = call;

        writer.stream() << "call " << call->function << std::endl;
        
        if(call->params > 0){
            allocateStackSpace(call->params);
        }

        if(call->return_){
            if(call->return_->type() == BaseType::FLOAT){
                float_registers.setLocation(call->return_, FloatRegister::XMM7);
            } else {
                registers.setLocation(call->return_, getReturnRegister1());
            }
                
            written.insert(call->return_);
        }

        if(call->return2_){
            registers.setLocation(call->return2_, getReturnRegister2());
            written.insert(call->return2_);
        }
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
    
    void compile(std::shared_ptr<tac::Param> param){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&param->arg)){
            if(!(*ptr)->type().isArray() && isFloatVar(*ptr)){
                Register reg = getReg();

                writer.stream() << getSizedMove() << reg << ", " << arg(*ptr) << std::endl;
                writer.stream() << "push " << reg << std::endl;

                registers.release(reg);
            } else {
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

                        writer.stream() << "mov " << reg << ", " << getBasePointerRegister() << std::endl;
                        writer.stream() << "add " << reg << ", " << -position.offset() << std::endl;
                        writer.stream() << "push " << reg << std::endl;

                        registers.release(reg);
                    } else if(position.isParameter()){
                        writer.stream() << "push " << getMnemonicSize() << " [" << getBasePointerRegister() << " + " << position.offset() << "]" << std::endl;
                    }
                } else {
                    writer.stream() << "push " << arg(param->arg) << std::endl;
                }
            }
        } else if(boost::get<double>(&param->arg)){
            Register reg = getReg();
            writer.stream() << "mov " << reg << ", " << arg(param->arg) << std::endl;
            writer.stream() << "push " << reg << std::endl;
            registers.release(reg);
        } else {
            writer.stream() << "push " << arg(param->arg) << std::endl;
        }
    }

    void compile(std::shared_ptr<tac::Quadruple> quadruple){
        current = quadruple;
        
        switch(quadruple->op){
            case tac::Operator::ASSIGN:
            {
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

                break;
            }
            case tac::Operator::FASSIGN:
            {
                //The fastest way to set a register to 0 is to use pxor
                if(tac::equals<int>(*quadruple->arg1, 0)){
                    FloatRegister reg = getFloatRegNoMove(quadruple->result);
                    writer.stream() << "pxor " << reg << ", " << reg << std::endl;            
                } 
                //In all the others cases, just move the value to the register
                else {
                    FloatRegister reg = getFloatRegNoMove(quadruple->result);
                    copy(*quadruple->arg1, reg);
                }

                written.insert(quadruple->result);

                break;
            }
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
                    writer.stream() << "imul " << arg(quadruple->result) << ", " << arg(*quadruple->arg1) << ", " << arg(*quadruple->arg2) << std::endl;
                }
                //Form x = y * z (y: immediate)
                else if(isInt(*quadruple->arg1) && isVariable(*quadruple->arg2)){
                    writer.stream() << "imul " << arg(quadruple->result) << ", " << arg(*quadruple->arg2) << ", " << arg(*quadruple->arg1) << std::endl;
                }
                //Form x = y * z (both variables)
                else if(isVariable(*quadruple->arg1) && isVariable(*quadruple->arg2)){
                    auto reg = getRegNoMove(quadruple->result);
                    copy(*quadruple->arg1, reg);
                    writer.stream() << "imul " << reg << ", " << arg(*quadruple->arg2) << std::endl;
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

                div(quadruple);

                written.insert(quadruple->result);
                
                break;            
            case tac::Operator::MOD:
                mod(quadruple);
                
                written.insert(quadruple->result);

                break;
            case tac::Operator::FADD:
            {
                auto result = quadruple->result;
                    
                //Optimize the special form a = a + b
                if(*quadruple->arg1 == result){
                    FloatRegister reg = getFloatReg(result);

                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatAdd() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatAdd() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                }
                //Optimize the special form a = b + a by using only one instruction
                else if(*quadruple->arg2 == result){
                    FloatRegister reg = getFloatReg(result);

                    if(tac::isFloat(*quadruple->arg1)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg1, reg2);
                        writer.stream() << getFloatAdd() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatAdd() << reg << ", " << arg(*quadruple->arg1) << std::endl;
                    }
                } 
                //In the other forms, use two instructions
                else {
                    FloatRegister reg = getFloatRegNoMove(result);
                    copy(*quadruple->arg1, reg);

                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatAdd() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatAdd() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                }
        
                written.insert(quadruple->result);

                break;
            }
            case tac::Operator::FSUB:
            {
                auto result = quadruple->result;

                //Optimize the special form a = a - b
                if(*quadruple->arg1 == result){
                    FloatRegister reg = getFloatReg(result);

                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatSub() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatSub() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                } else {
                    FloatRegister reg = getFloatRegNoMove(result);
                    copy(*quadruple->arg1, reg);

                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatSub() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatSub() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                }
                
                written.insert(quadruple->result);
                
                break;
            }
            case tac::Operator::FMUL:
                //Form  x = x * y
                if(*quadruple->arg1 == quadruple->result){
                    FloatRegister reg = getFloatReg(quadruple->result);

                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatMul() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatMul() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                }
                //Form x = y * x
                else if(*quadruple->arg2 == quadruple->result){
                    FloatRegister reg = getFloatReg(quadruple->result);
                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatMul() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatMul() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                } 
                //General form
                else  {
                    FloatRegister reg = getFloatRegNoMove(quadruple->result);
                    copy(*quadruple->arg1, reg);
                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatMul() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatMul() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                }
                
                written.insert(quadruple->result);

                break;            
            case tac::Operator::FDIV:
                //Form x = x / y
                if(*quadruple->arg1 == quadruple->result){
                    FloatRegister reg = getFloatReg(quadruple->result);
                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatDiv() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatDiv() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                } 
                //General form
                else {
                    FloatRegister reg = getFloatRegNoMove(quadruple->result);
                    copy(*quadruple->arg1, reg);
                    if(tac::isFloat(*quadruple->arg2)){
                        FloatRegister reg2 = getFloatReg();
                        copy(*quadruple->arg2, reg2);
                        writer.stream() << getFloatDiv() << reg << ", " << reg2 << std::endl;
                        float_registers.release(reg2);
                    } else {
                        writer.stream() << getFloatDiv() << reg << ", " << arg(*quadruple->arg2) << std::endl;
                    }
                }
                
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
                setIfCc("cmovg", quadruple);
                break;
            case tac::Operator::GREATER_EQUALS:
                setIfCc("cmovge", quadruple);
                break;
            case tac::Operator::LESS:
                setIfCc("cmovl", quadruple);
                break;
            case tac::Operator::LESS_EQUALS:
                setIfCc("cmovle", quadruple);
                break;
            case tac::Operator::EQUALS:
                setIfCc("cmove", quadruple);
                break;
            case tac::Operator::NOT_EQUALS:
                setIfCc("cmovne", quadruple);
                break;
            case tac::Operator::FG:
                setIfCc("cmova", quadruple);
                break;
            case tac::Operator::FGE:
                setIfCc("cmovae", quadruple);
                break;
            case tac::Operator::FL:
                setIfCc("cmovb", quadruple);
                break;
            case tac::Operator::FLE:
                setIfCc("cmovbe", quadruple);
                break;
            case tac::Operator::FE:
                setIfCc("cmove", quadruple);
                break;
            case tac::Operator::FNE:
                setIfCc("cmovne", quadruple);
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

                writer.stream() << "mov " << getMnemonicSize() << " " << toString(quadruple->result, offset) << ", " << arg(*quadruple->arg2) << std::endl;

                break;
            }
            case tac::Operator::ARRAY:
            {
                assert(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1));

                if(isFloatVar(quadruple->result)){
                    auto reg = getFloatRegNoMove(quadruple->result);

                    writer.stream() << getFloatMove() << reg << ", " << toString(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), *quadruple->arg2) << std::endl;
                } else {
                    auto reg = getRegNoMove(quadruple->result);

                    writer.stream() << "mov " << reg << ", " << toString(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1), *quadruple->arg2) << std::endl;
                }
               
                written.insert(quadruple->result);
                
                break;            
            }
            case tac::Operator::ARRAY_ASSIGN:
                if(quadruple->result->type() == BaseType::FLOAT){
                    auto reg = getFloatReg();

                    copy(*quadruple->arg2, reg);

                    writer.stream() << getFloatMove() << toString(quadruple->result, *quadruple->arg1) << ", " << reg << std::endl;

                    float_registers.release(reg);
                } else {
                    writer.stream() << "mov " << getMnemonicSize() << " " << toString(quadruple->result, *quadruple->arg1) << ", " << arg(*quadruple->arg2) << std::endl;
                }
                
                break;
            case tac::Operator::RETURN:
            {
                //A return without args is the same as exiting from the function
                if(quadruple->arg1){
                    if(isFloat(*quadruple->arg1)){
                        spills(FloatRegister::XMM7);
                        move(*quadruple->arg1, FloatRegister::XMM7);
                    } else if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1) && isFloatVar(boost::get<std::shared_ptr<Variable>>(*quadruple->arg1))){
                        auto variable = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);

                        FloatRegister reg = getFloatReg(variable);
                        if(reg != FloatRegister::XMM7){
                            spills(FloatRegister::XMM7);
                            writer.stream() << getFloatMove() << "xmm7, " << reg << std::endl;
                        }
                    } else {
                        Register reg1 = getReturnRegister1();
                        Register reg2 = getReturnRegister2();

                        spillsIfNecessary(reg1, *quadruple->arg1);

                        bool necessary = true;
                        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                            if(registers.inRegister(*ptr, reg1)){
                                necessary = false;
                            }
                        }    

                        if(necessary){
                            writer.stream() << "mov " << reg1 << ", " << arg(*quadruple->arg1) << std::endl;
                        }

                        if(quadruple->arg2){
                            spillsIfNecessary(reg2, *quadruple->arg2);

                            necessary = true;
                            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                                if(registers.inRegister(*ptr, reg2)){
                                    necessary = false;
                                }
                            }    

                            if(necessary){
                                writer.stream() << "mov " << reg2 << ", " << arg(*quadruple->arg2) << std::endl;
                            }
                        }
                    }
                }
    
                if(function->context->size() > 0){
                    deallocateStackSpace(function->context->size());
                }

                //The basic block must be ended before the jump
                endBasicBlock();

                writer.stream() << "leave" << std::endl;
                writer.stream() << "ret" << std::endl;

                break;
            }
        }
    }
};

} //end of as

} //end of eddic

#endif
