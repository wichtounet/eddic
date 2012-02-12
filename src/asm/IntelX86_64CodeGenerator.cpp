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

void as::IntelX86_64CodeGenerator::writeRuntimeSupport(FunctionTable& table){
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

void as::IntelX86_64CodeGenerator::addStandardFunctions(){
    //TODO
}

void as::IntelX86_64CodeGenerator::addGlobalVariables(std::shared_ptr<GlobalContext> context, StringPool& pool){
    //TODO
}

void as::IntelX86_64CodeGenerator::compile(std::shared_ptr<tac::Function> function){
    //TODO
}
