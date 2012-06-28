//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "assert.hpp"
#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"
#include "SymbolTable.hpp"
#include "Labels.hpp"
#include "VisitorUtils.hpp"

#include "asm/IntelX86_64CodeGenerator.hpp"
#include "asm/IntelAssemblyUtils.hpp"

using namespace eddic;

as::IntelX86_64CodeGenerator::IntelX86_64CodeGenerator(AssemblyFileWriter& w) : IntelCodeGenerator(w) {}

namespace x86_64 {

std::string to_string(eddic::ltac::Register reg){
    static std::string registers[14] = {
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};

    if(static_cast<int>(reg) == 1000){
        return "rsp"; 
    } else if(static_cast<int>(reg) == 1001){
        return "rbp"; 
    }

    return registers[static_cast<int>(reg)];
}

std::string to_string(eddic::ltac::FloatRegister reg){
    static std::string registers[8] = {
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"};

    return registers[static_cast<int>(reg)];
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

#include "to_address.inc"

std::ostream& operator<<(std::ostream& os, eddic::ltac::Argument& arg){
    if(auto* ptr = boost::get<int>(&arg)){
        return os << *ptr;
    } else if(auto* ptr = boost::get<double>(&arg)){
        return os << "__float64__(" << std::fixed << *ptr << ")";
    } else if(auto* ptr = boost::get<ltac::Register>(&arg)){
        return os << to_string(*ptr); 
    } else if(auto* ptr = boost::get<ltac::FloatRegister>(&arg)){
        return os << to_string(*ptr); 
    } else if(auto* ptr = boost::get<ltac::Address>(&arg)){
        return os << to_string(*ptr);
    } else if(auto* ptr = boost::get<std::string>(&arg)){
        return os << *ptr;
    }

    ASSERT_PATH_NOT_TAKEN("Unhandled variant type");
}
    
} //end of x86_64 namespace

using namespace x86_64;

namespace eddic { namespace as {

struct X86_64StatementCompiler : public boost::static_visitor<> {
    AssemblyFileWriter& writer;

    X86_64StatementCompiler(AssemblyFileWriter& writer) : writer(writer) {
        //Nothing else to init
    }

    void operator()(std::shared_ptr<ltac::Instruction> instruction){
        switch(instruction->op){
            case ltac::Operator::MOV:
                if(boost::get<ltac::FloatRegister>(&*instruction->arg1) && boost::get<ltac::Register>(&*instruction->arg2)){
                    writer.stream() << "movq " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                } else if(boost::get<ltac::Register>(&*instruction->arg1) && boost::get<ltac::FloatRegister>(&*instruction->arg2)){
                    writer.stream() << "movq " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                } else if(boost::get<ltac::Address>(&*instruction->arg1)){
                    writer.stream() << "mov qword " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                } else {
                    writer.stream() << "mov " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                }

                break;
            case ltac::Operator::FMOV:
                if(boost::get<ltac::FloatRegister>(&*instruction->arg1) && boost::get<ltac::Register>(&*instruction->arg2)){
                    writer.stream() << "movq " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                } else {
                    writer.stream() << "movsd " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                }

                break;
            case ltac::Operator::MEMSET:
                writer.stream() << "mov rcx, " << *instruction->arg2 << std::endl;
                writer.stream() << "xor rax, rax" << std::endl;
                writer.stream() << "lea rdi, " << *instruction->arg1 << std::endl;
                writer.stream() << "std" << std::endl;
                writer.stream() << "rep stosq" << std::endl;
                writer.stream() << "cld" << std::endl;

                break;
            case ltac::Operator::ALLOC_STACK:
                writer.stream() << "sub rsp, " << *instruction->arg1 << std::endl;
                break;
            case ltac::Operator::FREE_STACK:
                writer.stream() << "add rsp, " << *instruction->arg1 << std::endl;
                break;
            case ltac::Operator::LEAVE:
                leaveFunction(writer);
                break;
            case ltac::Operator::CMP_INT:
                writer.stream() << "cmp " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMP_FLOAT:
                writer.stream() << "ucomisd " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::OR:
                writer.stream() << "or " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::XOR:
                writer.stream() << "xor " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::PUSH:
                if(boost::get<ltac::Address>(&*instruction->arg1)){
                    writer.stream() << "push qword " << *instruction->arg1 << std::endl;
                } else {
                    writer.stream() << "push " << *instruction->arg1 << std::endl;
                }

                break;
            case ltac::Operator::POP:
                writer.stream() << "pop " << *instruction->arg1 << std::endl;
                break;
            case ltac::Operator::LEA:
                writer.stream() << "lea " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::SHIFT_LEFT:
                writer.stream() << "sal " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::SHIFT_RIGHT:
                writer.stream() << "sar " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::ADD:
                writer.stream() << "add " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::SUB:
                writer.stream() << "sub " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::MUL:
                if(instruction->arg3){
                    writer.stream() << "imul " << *instruction->arg1 << ", " << *instruction->arg2 << ", " << *instruction->arg3 << std::endl;
                } else {
                    writer.stream() << "imul " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                }

                break;
            case ltac::Operator::DIV:
                writer.stream() << "idiv " << *instruction->arg1 << std::endl;
                break;
            case ltac::Operator::FADD:
                writer.stream() << "addsd " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::FSUB:
                writer.stream() << "subsd " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::FMUL:
                writer.stream() << "mulsd " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::FDIV:
                writer.stream() << "divsd " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::INC:
                writer.stream() << "inc " << *instruction->arg1 << std::endl;
                break;
            case ltac::Operator::DEC:
                writer.stream() << "dec " << *instruction->arg1 << std::endl;
                break;
            case ltac::Operator::NEG:
                writer.stream() << "neg " << *instruction->arg1 << std::endl;
                break;
            case ltac::Operator::I2F:
                writer.stream() << "cvtsi2sd " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::F2I:
                writer.stream() << "cvttsd2si " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVE:
                writer.stream() << "cmove " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVNE:
                writer.stream() << "cmovne " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVA:
                writer.stream() << "cmova " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVAE:
                writer.stream() << "cmovae " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVB:
                writer.stream() << "cmovb " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVBE:
                writer.stream() << "cmovbe " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVG:
                writer.stream() << "cmovg " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVGE:
                writer.stream() << "cmovge " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVL:
                writer.stream() << "cmovl " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::CMOVLE:
                writer.stream() << "cmovle " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                break;
            case ltac::Operator::NOP:
                //Nothing to output for a nop
                break;
            default:
                ASSERT_PATH_NOT_TAKEN("The instruction operator is not supported");
        }
    }

    void operator()(std::shared_ptr<ltac::Jump> jump){
        switch(jump->type){
            case ltac::JumpType::CALL:
                writer.stream() << "call " << jump->label << std::endl;
                break;
            case ltac::JumpType::ALWAYS:
                writer.stream() << "jmp " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::NE:
                writer.stream() << "jne " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::E:
                writer.stream() << "je " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::GE:
                writer.stream() << "jge " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::G:
                writer.stream() << "jg " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::LE:
                writer.stream() << "jle " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::L:
                writer.stream() << "jl " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::AE:
                writer.stream() << "jae " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::A:
                writer.stream() << "ja" << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::BE:
                writer.stream() << "jbe " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::B:
                writer.stream() << "jb " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::P:
                writer.stream() << "jp " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::Z:
                writer.stream() << "jz " << "." << jump->label << std::endl;
                break;
            case ltac::JumpType::NZ:
                writer.stream() << "jnz " << "." << jump->label << std::endl;
                break;
            default:
                ASSERT_PATH_NOT_TAKEN("The jump type is not supported");
        }
    }

    void operator()(std::string& label){
        writer.stream() << "." << label << ":" << std::endl;
    }
};

void IntelX86_64CodeGenerator::compile(std::shared_ptr<ltac::Function> function){
    defineFunction(writer, function->getName());
    //TODO In the future, it is possible that it is up to the ltac compiler to generate the preamble of functions

    X86_64StatementCompiler compiler(writer);

    visit_each(compiler, function->getStatements());
}

void IntelX86_64CodeGenerator::writeRuntimeSupport(){
    writer.stream() << "section .text" << std::endl << std::endl;

    writer.stream() << "global _start" << std::endl << std::endl;

    writer.stream() << "_start:" << std::endl;

    //If the user wants the args, we add support for them
    if(symbols.getFunction("main")->parameters.size() == 1){
        writer.stream() << "pop rbx" << std::endl;                          //rbx = number of args
        
        //Calculate the size of the array
        writer.stream() << "mov rcx, rbx" << std::endl;
        writer.stream() << "imul rcx, rcx, 16" << std::endl;
        writer.stream() << "add rcx, 8" << std::endl;                       //rcx = size of the array

        writer.stream() << "push rcx" << std::endl;
        writer.stream() << "call eddi_alloc" << std::endl;                  //rax = start address of the array
        writer.stream() << "add rsp, 8" << std::endl;

        writer.stream() << "lea rsi, [rax + rcx - 16]" << std::endl;         //rsi = last address of the array
        writer.stream() << "mov rdx, rsi" << std::endl;                     //rdx = last address of the array
        
        writer.stream() << "mov [rsi], rbx" << std::endl;                   //Set the length of the array
        writer.stream() << "sub rsi, 16" << std::endl;                       //Move to the destination address of the first arg

        writer.stream() << ".copy_args:" << std::endl;
        writer.stream() << "pop rdi" << std::endl;                          //rdi = address of current args
        writer.stream() << "mov [rsi+8], rdi" << std::endl;                   //set the address of the string

        /* Calculate the length of the string  */
        writer.stream() << "xor rax, rax" << std::endl;
        writer.stream() << "xor rcx, rcx" << std::endl;
        writer.stream() << "not rcx" << std::endl;
        writer.stream() << "repne scasb" << std::endl;
        writer.stream() << "not rcx" << std::endl;
        writer.stream() << "dec rcx" << std::endl;
        /* End of the calculation */

        writer.stream() << "mov qword [rsi], rcx" << std::endl;               //set the length of the string
        writer.stream() << "sub rsi, 16" << std::endl;
        writer.stream() << "dec rbx" << std::endl;
        writer.stream() << "jnz .copy_args" << std::endl;

        writer.stream() << "push rdx" << std::endl;
    }

    //Give control to the user function
    writer.stream() << "call main" << std::endl;

    //Exit from the program
    writer.stream() << "mov rax, 60" << std::endl;  //syscall 60 is exit
    writer.stream() << "xor rdi, rdi" << std::endl; //exit code (0 = success)
    writer.stream() << "syscall" << std::endl;
}

void IntelX86_64CodeGenerator::defineDataSection(){
    writer.stream() << std::endl << "section .data" << std::endl;
}

void IntelX86_64CodeGenerator::declareIntArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "times " << size << " dq 0" << std::endl;
    writer.stream() << "dq " << size << std::endl;
}

void IntelX86_64CodeGenerator::declareFloatArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "times " << size << " dq __float64__(0.0)" << std::endl;
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

void IntelX86_64CodeGenerator::declareFloat(const std::string& label, double value){
    writer.stream() << label << std::fixed << " dq __float64__(" << value << ")" << std::endl;
}

}} //end of eddic::as

void output_function(AssemblyFileWriter& writer, const std::string& function){
    std::string name = "functions/" + function + ".s";
    std::ifstream stream(name.c_str());

    std::string str;

    while(!stream.eof()){
        std::getline(stream, str);

        if(!str.empty()){
            if(str[0] != ';'){
                writer.stream() << str << std::endl;
            }
        }
    }

    writer.stream() << std::endl;
}

void as::IntelX86_64CodeGenerator::addStandardFunctions(){
    if(as::is_enabled_printI()){
        output_function(writer, "x86_64_printI");
    }
   
    if(symbols.referenceCount("_F7printlnI")){
        output_function(writer, "x86_64_printlnI");
    }
    
    if(symbols.referenceCount("_F5printB")){
        output_function(writer, "x86_64_printB");
    }
    
    if(symbols.referenceCount("_F7printlnB")){
        output_function(writer, "x86_64_printlnB");
    }
    
    if(symbols.referenceCount("_F5printF")){
        output_function(writer, "x86_64_printF");
    }
    
    if(symbols.referenceCount("_F7printlnF")){
        output_function(writer, "x86_64_printlnF");
    }
    
    if(as::is_enabled_println()){
        output_function(writer, "x86_64_println");
    }
    
    if(symbols.referenceCount("_F5printS") || as::is_enabled_printI() || as::is_enabled_println()){ 
        output_function(writer, "x86_64_printS");
    }
   
    if(symbols.referenceCount("_F7printlnS")){ 
        output_function(writer, "x86_64_printlnS");
    }
    
    if(symbols.referenceCount("_F6concatSS")){
        output_function(writer, "x86_64_concat");
    }
    
    if(symbols.getFunction("main")->parameters.size() == 1 || symbols.referenceCount("_F6concatSS")){
        output_function(writer, "x86_64_eddi_alloc");
    }
    
    if(symbols.referenceCount("_F4timeAI")){
        output_function(writer, "x86_64_time");
    }
    
    if(symbols.referenceCount("_F8durationAIAI")){
        output_function(writer, "x86_64_duration");
    }
}
