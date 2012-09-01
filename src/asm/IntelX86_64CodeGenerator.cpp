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
#include "Labels.hpp"
#include "VisitorUtils.hpp"
#include "GlobalContext.hpp"

#include "asm/StringConverter.hpp"
#include "asm/IntelX86_64CodeGenerator.hpp"
#include "asm/IntelAssemblyUtils.hpp"

using namespace eddic;

as::IntelX86_64CodeGenerator::IntelX86_64CodeGenerator(AssemblyFileWriter& w, std::shared_ptr<GlobalContext> context) : IntelCodeGenerator(w, context) {}

namespace {

struct X86_64StringConverter : public as::StringConverter, public boost::static_visitor<std::string> {
    std::string operator()(ltac::Register& reg) const {
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
    
    std::string operator()(ltac::FloatRegister& reg) const {
        static std::string registers[8] = {
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"};

        return registers[static_cast<int>(reg)];
    }
    
    std::string operator()(ltac::Address& address) const {
        return address_to_string(address);
    }

    std::string operator()(int value) const {
       return ::toString(value);
    }

    std::string operator()(const std::string& value) const {
        return value;
    }

    std::string operator()(double value) const {
        std::stringstream ss;
        ss << "__float64__(" << std::fixed << value << ")";
        return ss.str();
    }
};

} //end of anonymous namespace

namespace x86_64 {

std::ostream& operator<<(std::ostream& os, eddic::ltac::Argument& arg){
    X86_64StringConverter converter;
    return os << visit(converter, arg);
}

} //end of x86_64 namespace

using namespace x86_64;

namespace {

struct X86_64StatementCompiler : public boost::static_visitor<> {
    AssemblyFileWriter& writer;

    X86_64StatementCompiler(AssemblyFileWriter& writer) : writer(writer) {
        //Nothing else to init
    }

    void operator()(std::shared_ptr<ltac::Instruction> instruction){
        switch(instruction->op){
            case ltac::Operator::MOV:
                if(instruction->size != ltac::Size::DEFAULT){
                    switch(instruction->size){
                        case ltac::Size::BYTE:
                            writer.stream() << "movzx " << *instruction->arg1 << ", byte " << *instruction->arg2 << std::endl;
                            break;
                        case ltac::Size::WORD:
                            writer.stream() << "movzx " << *instruction->arg1 << ", word " << *instruction->arg2 << std::endl;
                            break;
                        case ltac::Size::DOUBLE_WORD:
                            writer.stream() << "movzx " << *instruction->arg1 << ", dword " << *instruction->arg2 << std::endl;
                            break;
                        default:
                            writer.stream() << "mov " << *instruction->arg1 << ", qword " << *instruction->arg2 << std::endl;
                            break;
                    }

                    break;
                }

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
                writer.stream() << "rep stosq" << std::endl;

                break;
            case ltac::Operator::ENTER:
                writer.stream() << "push rbp" << std::endl;
                writer.stream() << "mov rbp, rsp" << std::endl;
                break;
            case ltac::Operator::LEAVE:
                writer.stream() << "mov rsp, rbp" << std::endl;
                writer.stream() << "pop rbp" << std::endl;
                break;
            case ltac::Operator::RET:
                writer.stream() << "ret" << std::endl;
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
            case ltac::Operator::NOT:
                writer.stream() << "not " << *instruction->arg1 << std::endl;
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

} //end of anonymous namespace

void as::IntelX86_64CodeGenerator::compile(std::shared_ptr<ltac::Function> function){
    writer.stream() << std::endl << function->getName() << ":" << std::endl;

    X86_64StatementCompiler compiler(writer);
    visit_each(compiler, function->getStatements());
}

void as::IntelX86_64CodeGenerator::writeRuntimeSupport(){
    writer.stream() << "section .text" << std::endl << std::endl;

    writer.stream() << "global _start" << std::endl << std::endl;

    writer.stream() << "_start:" << std::endl;
    
    //If necessary init memory manager 
    if(context->exists("_F4mainAS") || context->referenceCount("_F4freePI") || context->referenceCount("_F5allocI") || context->referenceCount("_F6concatSS")){
        writer.stream() << "call _F4init" << std::endl; 
    }

    //If the user wants the args, we add support for them
    if(context->exists("_F4mainAS")){
        writer.stream() << "pop rbx" << std::endl;                          //rbx = number of args
        
        //Calculate the size of the array
        writer.stream() << "mov rcx, rbx" << std::endl;
        writer.stream() << "imul rcx, rcx, 16" << std::endl;
        writer.stream() << "add rcx, 8" << std::endl;                       //rcx = size of the array

        writer.stream() << "mov r14, rcx" << std::endl;
        writer.stream() << "call _F5allocI" << std::endl;                   //rax = start address of the array

        writer.stream() << "mov rsi, rax" << std::endl;         //rsi = last address of the array
        writer.stream() << "mov rdx, rsi" << std::endl;                     //rdx = last address of the array
        
        writer.stream() << "mov [rsi], rbx" << std::endl;                   //Set the length of the array
        writer.stream() << "add rsi, 8" << std::endl;                       //Move to the destination address of the first arg

        writer.stream() << ".copy_args:" << std::endl;
        writer.stream() << "pop rdi" << std::endl;                          //rdi = address of current args
        writer.stream() << "mov [rsi], rdi" << std::endl;                   //set the address of the string

        /* Calculate the length of the string  */
        writer.stream() << "xor rax, rax" << std::endl;
        writer.stream() << "xor rcx, rcx" << std::endl;
        writer.stream() << "not rcx" << std::endl;
        writer.stream() << "repne scasb" << std::endl;
        writer.stream() << "not rcx" << std::endl;
        writer.stream() << "dec rcx" << std::endl;
        /* End of the calculation */

        writer.stream() << "mov [rsi+8], rcx" << std::endl;               //set the length of the string
        writer.stream() << "add rsi, 16" << std::endl;
        writer.stream() << "dec rbx" << std::endl;
        writer.stream() << "jnz .copy_args" << std::endl;

        writer.stream() << "push rdx" << std::endl;
    }

    //Give control to the user function
    if(context->exists("_F4mainAS")){
        writer.stream() << "call _F4mainAS" << std::endl;
    } else {
        writer.stream() << "call _F4main" << std::endl;
    }

    //Exit from the program
    writer.stream() << "mov rax, 60" << std::endl;  //syscall 60 is exit
    writer.stream() << "xor rdi, rdi" << std::endl; //exit code (0 = success)
    writer.stream() << "syscall" << std::endl;
}

void as::IntelX86_64CodeGenerator::defineDataSection(){
    writer.stream() << std::endl << "section .data" << std::endl;
}

void as::IntelX86_64CodeGenerator::declareIntArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "dq " << size << std::endl;
    writer.stream() << "times " << size << " dq 0" << std::endl;
}

void as::IntelX86_64CodeGenerator::declareFloatArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "dq " << size << std::endl;
    writer.stream() << "times " << size << " dq __float64__(0.0)" << std::endl;
}

void as::IntelX86_64CodeGenerator::declareStringArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<std::endl;
    writer.stream() << "dq " << size << std::endl;
    writer.stream() << "%rep " << size << std::endl;
    writer.stream() << "dq S3" << std::endl;
    writer.stream() << "dq 0" << std::endl;
    writer.stream() << "%endrep" << std::endl;
}

void as::IntelX86_64CodeGenerator::declareIntVariable(const std::string& name, int value){
    writer.stream() << "V" << name << " dq " << value << std::endl;
}

void as::IntelX86_64CodeGenerator::declareStringVariable(const std::string& name, const std::string& label, int size){
    writer.stream() << "V" << name << " dq " << label << ", " << size << std::endl;
}

void as::IntelX86_64CodeGenerator::declareString(const std::string& label, const std::string& value){
    writer.stream() << label << " dq " << value << std::endl;
}

void as::IntelX86_64CodeGenerator::declareFloat(const std::string& label, double value){
    writer.stream() << label << std::fixed << " dq __float64__(" << value << ")" << std::endl;
}

void as::IntelX86_64CodeGenerator::addStandardFunctions(){
    if(is_enabled_printI()){
        output_function("x86_64_printI");
    }
   
    if(context->referenceCount("_F7printlnI")){
        output_function("x86_64_printlnI");
    }
    
    if(context->referenceCount("_F5printC")){
        output_function("x86_64_printC");
    }
    
    if(context->referenceCount("_F7printlnC")){
        output_function("x86_64_printlnC");
    }
    
    if(context->referenceCount("_F5printB")){
        output_function("x86_64_printB");
    }
    
    if(context->referenceCount("_F7printlnB")){
        output_function("x86_64_printlnB");
    }
    
    if(context->referenceCount("_F5printF")){
        output_function("x86_64_printF");
    }
    
    if(context->referenceCount("_F7printlnF")){
        output_function("x86_64_printlnF");
    }
    
    if(is_enabled_println()){
        output_function("x86_64_println");
    }
    
    if(context->referenceCount("_F5printS") || is_enabled_printI() || is_enabled_println()){ 
        output_function("x86_64_printS");
    }
   
    if(context->referenceCount("_F7printlnS")){ 
        output_function("x86_64_printlnS");
    }
    
    if(context->referenceCount("_F6concatSS")){
        output_function("x86_64_concat");
    }
    
    //Memory management functions are included the three together
    if(context->exists("_F4mainAS") || context->referenceCount("_F4freePI") || context->referenceCount("_F5allocI") || context->referenceCount("_F6concatSS")){
        output_function("x86_64_alloc");
        output_function("x86_64_init");
        output_function("x86_64_free");
    }
    
    if(context->referenceCount("_F4timeAI")){
        output_function("x86_64_time");
    }
    
    if(context->referenceCount("_F8durationAIAI")){
        output_function("x86_64_duration");
    }
    
    if(context->referenceCount("_F9read_char")){
        output_function("x86_64_read_char");
    }
}
