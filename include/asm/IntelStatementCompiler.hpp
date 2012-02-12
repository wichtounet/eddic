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
#include <unordered_map>

#include "Utils.hpp"
#include "Registers.hpp"

#include "tac/Utils.hpp"

using namespace eddic;

namespace eddic {

namespace as {

template<typename Register>
struct IntelStatementCompiler {
    //The current function being compiled
    std::shared_ptr<tac::Function> function;
    
    AssemblyFileWriter& writer;
    Registers<Register> registers;

    std::unordered_map<std::shared_ptr<tac::BasicBlock>, std::string> labels;
    std::unordered_set<std::shared_ptr<tac::BasicBlock>> blockUsage;

    std::unordered_set<std::shared_ptr<Variable>> written;

    bool last;
    bool ended;

    tac::Statement current;
    tac::Statement next;
   
    IntelStatementCompiler(AssemblyFileWriter& w, std::vector<Register> r, std::shared_ptr<tac::Function> f) : function(f), writer(w), 
            registers(r, std::make_shared<Variable>("__fake__", Type(BaseType::INT, false), Position(PositionType::TEMPORARY))) {
        last = ended = false;        
    } 
   
    //Platform dependent (because of registers) 
    virtual std::string toString(std::shared_ptr<Variable> variable, tac::Argument offset) = 0;
    virtual std::string toString(std::shared_ptr<Variable> variable, int offset) = 0;
    
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
};

} //end of as

} //end of eddic

#endif
