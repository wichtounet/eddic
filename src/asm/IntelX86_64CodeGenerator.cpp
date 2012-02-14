//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"
#include "FunctionTable.hpp"
#include "Labels.hpp"
#include "VisitorUtils.hpp"

#include "asm/IntelStatementCompiler.hpp"
#include "asm/IntelX86_64CodeGenerator.hpp"

using namespace eddic;

as::IntelX86_64CodeGenerator::IntelX86_64CodeGenerator(AssemblyFileWriter& w) : IntelCodeGenerator(w) {}

namespace {

enum Register {
    RAX,
    RBX,
    RCX,
    RDX,

    R1, 
    R2, 
    R3,
    R4, 
    R5,
    R6,
    R7,
    R8,

    RSP, //Extended stack pointer
    RBP, //Extended base pointer

    RSI, //Extended source index
    RDI, //Extended destination index
    
    REGISTER_COUNT  
};

std::string regToString(Register reg){
    static std::string registers[Register::REGISTER_COUNT] = {
        "rax", "rbx", "rcx", "rdx", 
        "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
        "rsp", "rbp", "rsi", "rdi"};

    return registers[reg];
}

void enterFunction(AssemblyFileWriter& writer){
    writer.stream() << "push rbp" << std::endl;
    writer.stream() << "mov rbp, rsp" << std::endl;
}

void defineFunction(AssemblyFileWriter& writer, const std::string& function){
    writer.stream() << std::endl << function << ":" << std::endl;
    
    enterFunction(writer);
}

void leaveFunction(AssemblyFileWriter& writer){
    writer.stream() << "leave" << std::endl;
    writer.stream() << "ret" << std::endl;
}
    
} //end of anonymous namespace

namespace eddic { namespace as {

struct IntelX86_64StatementCompiler : public IntelStatementCompiler<Register>, public boost::static_visitor<> {
    IntelX86_64StatementCompiler(AssemblyFileWriter& w, std::shared_ptr<tac::Function> f) : IntelStatementCompiler(w, {RDI, RSI, RCX, RDX, R1, R2, R3, R4, R5, R6, R7, R8, RBX, RAX}, f) {}
    
    std::string getMnemonicSize(){
        return "qword";
    }

    Register getReturnRegister1(){
        return Register::RAX;
    }

    Register getReturnRegister2(){
        return Register::RBX;
    }

    Register getBasePointerRegister(){
        return Register::RBP;
    }

    Register getStackPointerRegister(){
        return Register::RSP;
    }
  
    //Div eax by arg2 
    void divEax(std::shared_ptr<tac::Quadruple> quadruple){
        writer.stream() << "mov rdx, rax" << std::endl;
        writer.stream() << "sar rdx, 31" << std::endl;

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
    
    void div(std::shared_ptr<tac::Quadruple> quadruple){
        spills(Register::RDX);
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

        registers.release(Register::RDX);
    }
    
    void mod(std::shared_ptr<tac::Quadruple> quadruple){
        spills(Register::RAX);
        spills(Register::RDX);

        registers.reserve(Register::RAX);
        registers.reserve(Register::RDX);

        copy(*quadruple->arg1, Register::RAX);

        divEax(quadruple);

        //result is in edx (no need to move it now)
        registers.setLocation(quadruple->result, Register::RDX);

        registers.release(Register::RAX);
    }
    
    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        compile(quadruple);
    }
    
    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        compile(ifFalse);
    }

    void operator()(std::shared_ptr<tac::If>& if_){
        compile(if_);
    }

    void operator()(std::shared_ptr<tac::Goto>& goto_){
        compile(goto_);
    }

    void operator()(std::shared_ptr<tac::Call>& call){
        compile(call);
    }

    void operator()(tac::NoOp&){
        //It's a no-op
    }

    void operator()(std::string&){
        assert(false); //There is no more label after the basic blocks have been extracted
    }
};

void as::IntelX86_64CodeGenerator::compile(std::shared_ptr<tac::Function> function){
    defineFunction(writer, function->getName());

    auto size = function->context->size();
    //Only if necessary, allocates size on the stack for the local variables
    if(size > 0){
        writer.stream() << "sub rsp, " << size << std::endl;
    }
    
    auto iter = function->context->begin();
    auto end = function->context->end();

    for(; iter != end; iter++){
        auto var = iter->second;
        if(var->type().isArray() && var->position().isStack()){
            int position = -var->position().offset();

            writer.stream() << "mov qword [rbp + " << position << "], " << var->type().size() << std::endl;

            if(var->type().base() == BaseType::INT){
                writer.stream() << "mov rcx, " << var->type().size() << std::endl;
            } else if(var->type().base() == BaseType::STRING){
                writer.stream() << "mov rcx, " << (var->type().size() * 2) << std::endl;
            }
            
            writer.stream() << "xor rax, rax" << std::endl;
            writer.stream() << "lea rdi, [rbp + " << position << " - 4]" << std::endl;
            writer.stream() << "std" << std::endl;
            writer.stream() << "rep stosq" << std::endl;
            writer.stream() << "cld" << std::endl;
        }
    }

    IntelX86_64StatementCompiler compiler(writer, function);

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

void as::IntelX86_64CodeGenerator::compile(std::shared_ptr<tac::BasicBlock> block, IntelX86_64StatementCompiler& compiler){
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

void IntelX86_64CodeGenerator::writeRuntimeSupport(FunctionTable& table){
    writer.stream() << "section .text" << std::endl << std::endl;

    writer.stream() << "global _start" << std::endl << std::endl;

    writer.stream() << "_start:" << std::endl;

    //If the user wants the args, we add support for them
    if(table.getFunction("main")->parameters.size() == 1){
        writer.stream() << "pop rbx" << std::endl;                          //ebx = number of args
        writer.stream() << "lea rcx, [4 + rbx * 8]" << std::endl;           //ecx = size of the array
        writer.stream() << "push rcx" << std::endl;
        writer.stream() << "call eddi_alloc" << std::endl;                  //eax = start address of the array
        writer.stream() << "add rsp, 4" << std::endl;

        writer.stream() << "lea rsi, [rax + rcx - 4]" << std::endl;         //esi = last address of the array
        writer.stream() << "mov rdx, rsi" << std::endl;                     //edx = last address of the array
        
        writer.stream() << "mov [rsi], rbx" << std::endl;                   //Set the length of the array
        writer.stream() << "sub rsi, 8" << std::endl;                       //Move to the destination address of the first arg

        writer.stream() << ".copy_args:" << std::endl;
        writer.stream() << "pop rdi" << std::endl;                          //edi = address of current args
        writer.stream() << "mov [rsi+4], rdi" << std::endl;                 //set the address of the string

        /* Calculate the length of the string  */
        writer.stream() << "xor rax, rax" << std::endl;
        writer.stream() << "xor rcx, rcx" << std::endl;
        writer.stream() << "not rcx" << std::endl;
        writer.stream() << "repne scasb" << std::endl;
        writer.stream() << "not rcx" << std::endl;
        writer.stream() << "dec rcx" << std::endl;
        /* End of the calculation */

        writer.stream() << "mov dword [rsi], ecx" << std::endl;               //set the length of the string
        writer.stream() << "sub rsi, 8" << std::endl;
        writer.stream() << "dec erx" << std::endl;
        writer.stream() << "jnz .copy_args" << std::endl;

        writer.stream() << "push rdx" << std::endl;
    }

    writer.stream() << "call main" << std::endl;
    writer.stream() << "mov rax, 1" << std::endl;
    writer.stream() << "xor rbx, rbx" << std::endl;
    writer.stream() << "int 80h" << std::endl;
}

void IntelX86_64CodeGenerator::defineDataSection(){
    writer.stream() << std::endl << "section .data" << std::endl;
}

void IntelX86_64CodeGenerator::declareIntArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "%rep " << size << std::endl;
    writer.stream() << "dq 0" << std::endl;
    writer.stream() << "%endrep" << std::endl;
    writer.stream() << "dq " << size << std::endl;
}

void IntelX86_64CodeGenerator::declareStringArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "%rep " << size << std::endl;
    writer.stream() << "dq S3" << std::endl;
    writer.stream() << "dq 0" << std::endl;
    writer.stream() << "%endrep" << std::endl;
    writer.stream() << "dq " << size << std::endl;
}

void IntelX86_64CodeGenerator::declareIntVariable(const std::string& name, int value){
    writer.stream() << "V" << name << " dq " << value << std::endl;
}

void IntelX86_64CodeGenerator::declareStringVariable(const std::string& name, const std::string& label, int size){
    writer.stream() << "V" << name << " dq " << label << ", " << size << std::endl;
}

void IntelX86_64CodeGenerator::declareString(const std::string& label, const std::string& value){
    writer.stream() << label << " dq " << value << std::endl;
}

void IntelX86_64CodeGenerator::addStandardFunctions(){
    //TODO
}

}} //end of eddic::as
