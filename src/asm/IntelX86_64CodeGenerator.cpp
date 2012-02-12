//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "AssemblyFileWriter.hpp"
#include "FunctionTable.hpp"

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

void IntelX86_64CodeGenerator::compile(std::shared_ptr<tac::Function> function){
    //TODO
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
