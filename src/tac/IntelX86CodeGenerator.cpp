//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <memory>
#include <unordered_map>

#include <boost/variant.hpp>

#include "tac/IntelX86CodeGenerator.hpp"

#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"

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
    
    std::shared_ptr<Variable> descriptors[Register::REGISTER_COUNT];
    std::unordered_map<std::shared_ptr<Variable>, Register> variables;

    StatementCompiler(AssemblyFileWriter& w, std::shared_ptr<tac::Function> f) : writer(w), function(f) {}

    std::string arg(tac::Argument argument){
        if(auto* ptr = boost::get<int>(&argument)){
            return "$" + toString(*ptr);
        } else if(auto* ptr = boost::get<std::string>(&argument)){
            return "$" + *ptr;
        } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&argument)){
            if(variables.find(*ptr) != variables.end()){
                //The variables is already in a register
                return regToString(variables[*ptr]);
            } else {
               //The variable is not in a register
                
                return "";
            }
        }

        assert(false);
    }

    void spills(Register reg){
        //TODO Spills the content of the reg with the valid content
    }

    void operator()(std::shared_ptr<tac::Goto>& goto_){
       writer.stream() << "goto " << labels[goto_->block] << std::endl; 
    }

    void operator()(std::shared_ptr<tac::Param>& param){
        writer.stream() << "pushl " << arg(param->arg) << std::endl;
    }

    void operator()(std::shared_ptr<tac::Call>& call){
        writer.stream() << "call " << call->function << std::endl;
        
        if(call->params > 0){
            writer.stream() << "addl " << call->params << ", %esp" << std::endl;
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
    
    void operator()(std::shared_ptr<tac::Return>& return_){
        //A return without args is the same as exiting from the function
        if(return_->arg1){
            if(descriptors[Register::EAX]){
                spills(Register::EAX);
            }
            
            writer.stream() << "movl " << arg(*return_->arg1) << ", %eax" << std::endl;

            if(return_->arg2){
                if(descriptors[Register::EBX]){
                    spills(Register::EBX);
                }

                writer.stream() << "movl " << arg(*return_->arg2) << ", %ebx" << std::endl;
            }
        }
        
        if(function->context->size() > 0){
            writer.stream() << "addl $" << function->context->size() << " , %esp" << std::endl;
        }

        writer.stream() << "leave" << std::endl;
        writer.stream() << "ret" << std::endl;
    }
    
    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        //TODO
    }
    
    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        writer.stream() << "cmpl " << arg(ifFalse->arg1) << ", " << arg(ifFalse->arg2) << std::endl;

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
    std::string label = newLabel();

    compiler.labels[block] = label;

    writer.stream() << label << ":" << std::endl;

    for(auto& statement : block->statements){
        boost::apply_visitor(compiler, statement);
    }
}

bool updateLiveness(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, tac::Argument arg){
    if(auto* variable = boost::get<std::shared_ptr<Variable>>(&arg)){
        if(liveness.find(*variable) != liveness.end()){
            if((*variable)->position().isGlobal()){
                liveness[*variable] = true;
            } else {
                liveness[*variable] = false;
            }
        }

        bool live = liveness[*variable];

        //variable is live
        liveness[*variable] = true;

        return live;
    }

    return false;
}

void tac::IntelX86CodeGenerator::computeLiveness(std::shared_ptr<tac::Function> function){
    std::vector<std::shared_ptr<BasicBlock>>::reverse_iterator bit = function->getBasicBlocks().rbegin();
    std::vector<std::shared_ptr<BasicBlock>>::reverse_iterator bend = function->getBasicBlocks().rend(); 
    
    std::unordered_map<std::shared_ptr<Variable>, bool> liveness;

    while(bit != bend){
        std::vector<tac::Statement>::reverse_iterator sit = (*bit)->statements.rbegin();
        std::vector<tac::Statement>::reverse_iterator send = (*bit)->statements.rend(); 

        while(sit != send){
            auto statement = *sit;

            if(auto* ptr = boost::get<std::shared_ptr<tac::Param>>(&statement)){
                (*ptr)->liveVariable = updateLiveness(liveness, (*ptr)->arg);
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::Return>>(&statement)){
                if((*ptr)->arg1){
                    (*ptr)->liveVariable1 = updateLiveness(liveness, (*(*ptr)->arg1));
                }
                
                if((*ptr)->arg2){
                    (*ptr)->liveVariable2 = updateLiveness(liveness, (*(*ptr)->arg2));
                }
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::IfFalse>>(&statement)){
                (*ptr)->liveVariable1 = updateLiveness(liveness, (*ptr)->arg1);
                (*ptr)->liveVariable2 = updateLiveness(liveness, (*ptr)->arg2);
            } else if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&statement)){
                (*ptr)->liveVariable1 = updateLiveness(liveness, (*ptr)->arg1);
                
                if((*ptr)->arg2){
                    (*ptr)->liveVariable2 = updateLiveness(liveness, (*(*ptr)->arg2));
                }

                (*ptr)->liveResult = liveness[(*ptr)->result];

                liveness[(*ptr)->result] = false; 
            }

            sit++;
        }

        bit++;
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

    StatementCompiler compiler(writer, function);
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

void tac::IntelX86CodeGenerator::generate(tac::Program& program){
    resetNumbering();

    for(auto& function : program.functions){
        compile(function);
    }
}
