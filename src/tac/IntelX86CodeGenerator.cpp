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

#include "tac/IntelX86CodeGenerator.hpp"
#include "tac/Printer.hpp"

#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"
#include "GlobalContext.hpp"
#include "StringPool.hpp"

#include "il/Labels.hpp"

using namespace eddic;

tac::IntelX86CodeGenerator::IntelX86CodeGenerator(AssemblyFileWriter& w) : writer(w) {}

namespace eddic { namespace tac { 

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

    std::unordered_map<std::shared_ptr<BasicBlock>, std::string> labels;
    std::unordered_set<std::shared_ptr<BasicBlock>> blockUsage;
 
    std::vector<Register> registers;   
    std::shared_ptr<Variable> descriptors[Register::REGISTER_COUNT];
    std::unordered_map<std::shared_ptr<Variable>, Register> variables;

    tac::Statement current;

    std::shared_ptr<Variable> retainVariable;

    bool last;
    bool ended;

    StatementCompiler(AssemblyFileWriter& w, std::shared_ptr<tac::Function> f) : writer(w), function(f) {
        registers = {EDI, ESI, ECX, EDX, EBX, EAX};

        retainVariable = std::make_shared<Variable>("__fake__", Type(BaseType::INT), Position(PositionType::TEMPORARY));

        last = ended = false;
    }

    //Called at the beginning of each basic block
    void reset(){
        variables.clear();

        for(unsigned int i = 0; i < Register::REGISTER_COUNT; ++i){
            descriptors[i] = nullptr;
        }

        last = ended = false;
    }

    bool isLive(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, std::shared_ptr<Variable> variable){
       if(liveness.find(variable) != liveness.end()){
            return liveness[variable];   
       } else {
            if(variable->position().isTemporary()){
                return false;
            } else {
                return true;
            }
       }
    }

    bool isLive(std::shared_ptr<Variable> variable){
        if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&current)){
            return isLive((*ptr)->liveness, variable);
        } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&current)){
            return isLive((*ptr)->liveness, variable);
        } else if(auto* ptr = boost::get<std::shared_ptr<tac::Return>>(&current)){
            return isLive((*ptr)->liveness, variable);
        }

        assert(false); //No liveness calculations in the other cases
    }

    void move(tac::Argument argument, Register reg){
        if(auto* ptr = boost::get<int>(&argument)){
            writer.stream() << "movl $" << ::toString(*ptr) << ", " << regToString(reg) << std::endl;
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            writer.stream() << "movl $" << *ptr << ", " << regToString(reg) << std::endl;
        } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            auto variable = *ptr;
            auto position = variable->position();

            if(position.isStack()){
                writer.stream() << "movl " << (-1 * position.offset()) << "(%ebp), " << regToString(reg) << std::endl; 
            } else if(position.isParameter()){
                writer.stream() << "movl " << position.offset() << "(%ebp), " << regToString(reg) << std::endl; 
            } else if(position.isGlobal()){
                writer.stream() << "movl " << "V" << position.name() << ", " << regToString(reg) << std::endl;
            } else if(position.isTemporary()){
                assert(variables.find(variable) != variables.end());

                auto oldReg = variables[variable];
                
                writer.stream() << "movl " << regToString(oldReg) << ", " << regToString(reg) << std::endl;
            }
        }
    }

    void spills(Register reg){
        //If the register is not used, there is nothing to spills
        if(descriptors[reg]){
            auto variable = descriptors[reg];
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
                    //Remove it from the register
                    descriptors[reg] = retainVariable;
                    variables.erase(variable);
                    
                    Register newReg = getReg(variable, false);
                    writer.stream() << "movl " << regToString(reg) << ", " << regToString(newReg) << std::endl;
                
                    assert(newReg != reg);
                    assert(descriptors[newReg] == variable);
                    assert(variables[variable] = newReg);

                    descriptors[reg] = nullptr;
                    return; //Return here to avoid erasing variable from variables
                }
            }
            
            //The variable is no more contained in the register
            descriptors[reg] = nullptr;
            variables.erase(variable);
        }
    }
    
    Register getReg(std::shared_ptr<Variable> variable, bool doMove){
        //The variable is already in a register
        if(variables.find(variable) != variables.end()){
            return variables[variable];
        }
       
        //Try to get a free register 
        for(auto reg : registers){
            if(!descriptors[reg] || (descriptors[reg] != retainVariable && !isLive(descriptors[reg]))){
                if(doMove){
                    move(variable, reg);
                }

                descriptors[reg] = variable;
                variables[variable] = reg;

                return reg;
            }
        }

        //There are no free register, take one
        auto reg = registers[0];
        spills(reg);

        if(doMove){
            move(variable, reg);
        }

        descriptors[reg] = variable;
        variables[variable] = reg;

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
            if(!descriptors[reg] || (descriptors[reg] != retainVariable && !isLive(descriptors[reg]))){
                variables.erase(descriptors[reg]);
                descriptors[reg] = retainVariable;

                return reg;
            }
        }

        //There are no free register, take one
        auto reg = registers[0];
        spills(reg);

        variables.erase(descriptors[reg]);
        descriptors[reg] = retainVariable;

        return reg;
    }
    
    std::string toString(std::shared_ptr<Variable> variable, int offset){
        auto position = variable->position();

        if(position.isStack()){
            return ::toString(-position.offset() + offset) + "(%ebp)";
        } else if(position.isParameter()){
            //TODO This register allocation is not safe
            Register reg = getReg();
            
            writer.stream() << "movl " << ::toString(position.offset()) << "(%ebp), " << regToString(reg) << std::endl;

            descriptors[reg] = nullptr;

            return ::toString(offset) + "(" + regToString(reg)  + ")";
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

            descriptors[reg] = nullptr;

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
            descriptors[Register::EAX] = call->return_;
            variables[call->return_] = Register::EAX;
        }

        if(call->return2_){
            descriptors[Register::EBX] = call->return2_;
            variables[call->return2_] = Register::EBX;
        }
    }
   
    void spillsIfNecessary(Register reg, tac::Argument arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
            if(descriptors[reg] != *ptr){
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
        for(unsigned int i = 0; i < Register::REGISTER_COUNT; ++i){
            if(descriptors[i]){
                auto variable = descriptors[i];

                if(!variable->position().isTemporary()){
                    spills((Register)i);    
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
                if(variables.find(*ptr) != variables.end()){
                    if(variables[*ptr] == Register::EAX){
                        necessary = false;
                    }
                }
            }    

            if(necessary){
                writer.stream() << "movl " << arg(*return_->arg1) << ", %eax" << std::endl;
            }

            if(return_->arg2){
                spillsIfNecessary(Register::EBX, *return_->arg2);
                
                necessary = true;
                if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*return_->arg2)){
                    if(variables.find(*ptr) != variables.end()){
                        if(variables[*ptr] == Register::EBX){
                            necessary = false;
                        }
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
    
    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        current = quadruple;
        
        if(!quadruple->op){
            //TODO Optimize move of 0 in a register with xorl
            Register reg = getRegNoMove(quadruple->result);
            writer.stream() << "movl " << arg(quadruple->arg1) << ", " << regToString(reg) << std::endl;            
        } else {
            switch(*quadruple->op){
                //TODO Optimize a = a + 1 and a = a -1 with inc and dec
                //TODO Find a way to optimize statements like a = a + b or a = b + a
                case Operator::ADD:
                {
                    auto result = quadruple->result;

                    //If the firsrt arg is the same variable as the result : a = a + x
                    if(boost::get<std::shared_ptr<Variable>>(&quadruple->arg1) && boost::get<std::shared_ptr<Variable>>(quadruple->arg1) == result){
                        Register reg = getReg(quadruple->result);
                        writer.stream() << "addl " << arg(*quadruple->arg2) << ", " << regToString(reg) << std::endl;
                    } else if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2) && boost::get<std::shared_ptr<Variable>>(*quadruple->arg2) == result){
                        Register reg = getReg(quadruple->result);
                        writer.stream() << "addl " << arg(quadruple->arg1) << ", " << regToString(reg) << std::endl;
                    } else {                   
                        Register reg = getRegNoMove(quadruple->result);
                        writer.stream() << "movl " << arg(quadruple->arg1) << ", " << regToString(reg) << std::endl;
                        writer.stream() << "addl " << arg(*quadruple->arg2) << ", " << regToString(reg) << std::endl;
                    }

                    break;
                }
                case Operator::SUB:
                {
                    Register reg = getRegNoMove(quadruple->result);
                    writer.stream() << "movl " << arg(quadruple->arg1) << ", " << regToString(reg) << std::endl;
                    writer.stream() << "subl " << arg(*quadruple->arg2) << ", " << regToString(reg) << std::endl;
                    break;
                }
                //TODO Simplify this generation
                case Operator::MUL:
                {
                    bool fast = false;

                    //Form x = -1 * x
                    if(boost::get<int>(&quadruple->arg1) && boost::get<int>(quadruple->arg1) == -1 && 
                        boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2) && boost::get<std::shared_ptr<Variable>>(*quadruple->arg2) == quadruple->result){

                        writer.stream() << "neg " << arg(quadruple->result) << std::endl;

                        fast = true;
                    } 
                    //Form x = x * -1
                    else if(boost::get<int>(&*quadruple->arg2) && boost::get<int>(*quadruple->arg2) == -1 && 
                        boost::get<std::shared_ptr<Variable>>(&quadruple->arg1) && boost::get<std::shared_ptr<Variable>>(quadruple->arg1) == quadruple->result){
                        
                        writer.stream() << "neg " << arg(quadruple->result) << std::endl;

                        fast = true;
                    }

                    //If arg 1 is in eax
                    if(!fast){
                        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&quadruple->arg1)){
                            if(descriptors[Register::EAX] == *ptr){
                                if((*ptr)->position().isTemporary() && !quadruple->liveness[*ptr]){
                                    //If the arg is a variable, it will be matched to a register automatically
                                    if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2))
                                    {
                                        writer.stream() << "mull " << arg(*quadruple->arg2) << std::endl;
                                    } //If it's an immediate value, we have to move it in a register
                                    else if (boost::get<int>(&*quadruple->arg2)){
                                        auto reg = getReg();

                                        move(*quadruple->arg2, reg);
                                        writer.stream() << "mull " << regToString(reg) << std::endl;

                                        descriptors[reg] = nullptr;
                                    }

                                    fast = true;
                                }
                            }
                        }
                    }

                    //If arg 2 is in eax
                    if(!fast){
                        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2)){
                            if(descriptors[Register::EAX] == *ptr){
                                if((*ptr)->position().isTemporary() && !quadruple->liveness[*ptr]){
                                    //If the arg is a variable, it will be matched to a register automatically
                                    if(boost::get<std::shared_ptr<Variable>>(&quadruple->arg1))
                                    {
                                        writer.stream() << "mull " << arg(quadruple->arg1) << std::endl;
                                    } //If it's an immediate value, we have to move it in a register
                                    else if (boost::get<int>(&quadruple->arg1)){
                                        auto reg = getReg();

                                        move(quadruple->arg1, reg);
                                        writer.stream() << "mull " << regToString(reg) << std::endl;

                                        descriptors[reg] = nullptr;
                                    }

                                    fast = true;
                                }
                            }
                        }
                    }

                    //Neither of the args is in a register
                    if(!fast){
                        spills(Register::EAX);

                        move(quadruple->arg1, Register::EAX);

                        descriptors[Register::EAX] = retainVariable;
                        
                        //If the arg is a variable, it will be matched to a register automatically
                        if(boost::get<std::shared_ptr<Variable>>(&*quadruple->arg2))
                        {
                            writer.stream() << "mull " << arg(*quadruple->arg2) << std::endl;
                        } //If it's an immediate value, we have to move it in a register
                        else if (boost::get<int>(&*quadruple->arg2)){
                            auto reg = getReg();

                            move(*quadruple->arg2, reg);
                            writer.stream() << "mull " << regToString(reg) << std::endl;

                            descriptors[reg] = nullptr;
                        }
                    }

                    //result is in eax (no need to move it now)
                    descriptors[Register::EAX] = quadruple->result;
                    variables[quadruple->result] = Register::EAX;
                    break;            
                }
                case Operator::DIV:
                    spills(Register::EAX);

                    move(quadruple->arg1, Register::EAX);

                    writer.stream() << "divl " << arg(*quadruple->arg2) << std::endl;

                    //result is in eax (no need to move it now)
                    descriptors[Register::EAX] = quadruple->result;
                    variables[quadruple->result] = Register::EAX;
                    break;            
                case Operator::MOD:
                    spills(Register::EAX);
                    spills(Register::EDX);

                    move(quadruple->arg1, Register::EAX);

                    writer.stream() << "divl " << arg(*quadruple->arg2) << std::endl;

                    //result is in edx (no need to move it now)
                    descriptors[Register::EDX] = quadruple->result;
                    variables[quadruple->result] = Register::EDX;
                    break;            
                case Operator::DOT:
                {
                   assert(boost::get<std::shared_ptr<Variable>>(&quadruple->arg1));
                   assert(boost::get<int>(&*quadruple->arg2));

                   int offset = boost::get<int>(*quadruple->arg2);
                   auto variable = boost::get<std::shared_ptr<Variable>>(quadruple->arg1);
   
                   Register reg = getRegNoMove(quadruple->result);
                   writer.stream() << "movl " << toString(variable, offset) << ", " << regToString(reg) << std::endl;
                   break;
                }
                case Operator::DOT_ASSIGN:
                {
                    assert(boost::get<int>(&quadruple->arg1));

                    int offset = boost::get<int>(quadruple->arg1);

                    writer.stream() << "movl " << arg(*quadruple->arg2) << ", " << toString(quadruple->result, offset) << std::endl;
                    break;
                }
                case Operator::ARRAY:
                {
                    assert(boost::get<std::shared_ptr<Variable>>(&quadruple->arg1));

                    Register reg = getRegNoMove(quadruple->result);
            
                    writer.stream() << "movl " << toString(boost::get<std::shared_ptr<Variable>>(quadruple->arg1), *quadruple->arg2) << ", " << regToString(reg) << std::endl;
                    break;            
                }
                case Operator::ARRAY_ASSIGN:
                    writer.stream() << "movl " << arg(*quadruple->arg2) << ", " << toString(quadruple->result, quadruple->arg1) << std::endl;
                    break;
                case Operator::PARAM:
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

                                descriptors[reg] = nullptr;
                            } else if(position.isStack()){
                                Register reg = getReg();

                                writer.stream() << "movl %ebp, " << regToString(reg) << std::endl;
                                writer.stream() << "addl $" << (-position.offset()) << ", " << regToString(reg) << std::endl;
                                writer.stream() << "pushl " << regToString(reg) << std::endl;
                                
                                descriptors[reg] = nullptr;
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
    
    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        current = ifFalse;

        //The first argument is not important, it can be immediate, but the second must be a register
        if(auto* ptr = boost::get<int>(&ifFalse->arg1)){
            auto reg = getReg();

            writer.stream() << "movl $" << *ptr << ", " << regToString(reg) << std::endl;
            
            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "cmpl " << arg(ifFalse->arg2) << ", " << regToString(reg) << std::endl;

            descriptors[reg] = nullptr;
        } else {
            //The basic block must be ended before the jump
            endBasicBlock();

            writer.stream() << "cmpl " << arg(ifFalse->arg2) << ", " << arg(ifFalse->arg1) << std::endl;
        }

        switch(ifFalse->op){
            case BinaryOperator::EQUALS:
                writer.stream() << "jne " << labels[ifFalse->block] << std::endl;
                break;
            case BinaryOperator::NOT_EQUALS:
                writer.stream() << "je " << labels[ifFalse->block] << std::endl;
                break;
            case BinaryOperator::LESS:
                writer.stream() << "jge " << labels[ifFalse->block] << std::endl;
                break;
            case BinaryOperator::LESS_EQUALS:
                writer.stream() << "jg " << labels[ifFalse->block] << std::endl;
                break;
            case BinaryOperator::GREATER:
                writer.stream() << "jle " << labels[ifFalse->block] << std::endl;
                break;
            case BinaryOperator::GREATER_EQUALS:
                writer.stream() << "jl " << labels[ifFalse->block] << std::endl;
                break;
        }
    }

    void operator()(std::string&){
        assert(false); //There is no more label after the basic blocks have been extracted
    }
};

}}

void tac::IntelX86CodeGenerator::compile(std::shared_ptr<tac::BasicBlock> block, StatementCompiler& compiler){
    compiler.reset();

    if(compiler.blockUsage.find(block) != compiler.blockUsage.end()){
        writer.stream() << compiler.labels[block] << ":" << std::endl;
    }

    for(unsigned int i = 0; i < block->statements.size(); ++i){
        auto& statement = block->statements[i];

        if(i == block->statements.size() - 1){
            compiler.setLast(true);
        }
        
        boost::apply_visitor(compiler, statement);
    }

    //If the basic block has not been ended
    if(!compiler.ended){
        compiler.endBasicBlock();
    }
}

//Set the default properties of the variable
void updateLive(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, tac::Argument arg){
    if(auto* variable = boost::get<std::shared_ptr<Variable>>(&arg)){
        if(liveness.find(*variable) == liveness.end()){
            if((*variable)->position().isTemporary()){
                liveness[*variable] = false;
            } else {
                liveness[*variable] = true;
            }
        }
    }
}

//Set the variable as live
void setLive(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, tac::Argument arg){
    if(auto* variable = boost::get<std::shared_ptr<Variable>>(&arg)){
        liveness[*variable] = true;
    }
}

void tac::IntelX86CodeGenerator::computeLiveness(std::shared_ptr<tac::Function> function){
    std::vector<std::shared_ptr<BasicBlock>>::reverse_iterator bit = function->getBasicBlocks().rbegin();
    std::vector<std::shared_ptr<BasicBlock>>::reverse_iterator bend = function->getBasicBlocks().rend(); 
    
    std::unordered_map<std::shared_ptr<Variable>, bool> liveness;

    while(bit != bend){
        std::vector<tac::Statement>::reverse_iterator sit = (*bit)->statements.rbegin();
        std::vector<tac::Statement>::reverse_iterator send = (*bit)->statements.rend(); 
    
        liveness.clear();

        while(sit != send){
            auto statement = *sit;

            if(auto* ptr = boost::get<std::shared_ptr<tac::Return>>(&statement)){
                if((*ptr)->arg1){
                    updateLive(liveness, (*(*ptr)->arg1));
                }
                
                if((*ptr)->arg2){
                    updateLive(liveness, (*(*ptr)->arg2));
                }
                
                if((*ptr)->arg1){
                    setLive(liveness, (*(*ptr)->arg1));
                }
                
                if((*ptr)->arg2){
                    setLive(liveness, (*(*ptr)->arg2));
                }
               
                (*ptr)->liveness = liveness;
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                updateLive(liveness, (*ptr)->arg1);
                updateLive(liveness, (*ptr)->arg2);
                
                setLive(liveness, (*ptr)->arg1);
                setLive(liveness, (*ptr)->arg2);
                
                (*ptr)->liveness = liveness;
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&statement)){
                updateLive(liveness, (*ptr)->arg1);
                
                if((*ptr)->arg2){
                    updateLive(liveness, (*(*ptr)->arg2));
                }
                
                setLive(liveness, (*ptr)->arg1);
                
                if((*ptr)->arg2){
                    setLive(liveness, (*(*ptr)->arg2));
                }
                
                (*ptr)->liveness = liveness;

                //TODO Verify if this point should make before assigning livenes to the statement
                if((*ptr)->result){
                    liveness[(*ptr)->result] = false;
                }
            }

            sit++;
        }

        bit++;
    }
}

void tac::IntelX86CodeGenerator::computeBlockUsage(std::shared_ptr<tac::Function> function, StatementCompiler& compiler){
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<tac::Goto>>(&statement)){
                compiler.blockUsage.insert((*ptr)->block);
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                compiler.blockUsage.insert((*ptr)->block);
            }
        }
    }
}

void tac::IntelX86CodeGenerator::compile(std::shared_ptr<tac::Function> function){
    computeLiveness(function);

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

    computeBlockUsage(function, compiler);

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

void tac::IntelX86CodeGenerator::writeRuntimeSupport(){
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

void tac::IntelX86CodeGenerator::addStandardFunctions(){
   addPrintIntegerFunction(writer); 
   addPrintLineFunction(writer); 
   addPrintStringFunction(writer); 
   addConcatFunction(writer);
   addAllocFunction(writer);
}

void tac::IntelX86CodeGenerator::addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool){
    writer.stream() << std::endl << ".data" << std::endl;
     
    for(auto it : context->getVariables()){
        Type type = it.second->type();

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
    
                writer.stream() << ".size V" << it.second->position().name() << ", 8" << std::endl;
                writer.stream() << "V" << it.second->position().name() << ":" << std::endl;
                writer.stream() << ".long " << value.first << std::endl;
                writer.stream() << ".long " << value.second << std::endl;
            }
        }
    }
    
    for (auto it : pool.getPool()){
        writer.stream() << it.second << ":" << std::endl;
        writer.stream() << ".string " << it.first << std::endl;
    }
}

void tac::IntelX86CodeGenerator::generate(tac::Program& program, StringPool& pool){
    writeRuntimeSupport(); 

    resetNumbering();

    for(auto& function : program.functions){
        compile(function);
    }

    addStandardFunctions();

    addGlobalVariables(program.context, pool);
}
