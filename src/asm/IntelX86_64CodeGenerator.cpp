//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"
#include "SymbolTable.hpp"
#include "Labels.hpp"
#include "VisitorUtils.hpp"

#include "asm/IntelStatementCompiler.hpp"
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

std::string to_string(eddic::ltac::Address& address){
    if(address.absolute){
        if(address.displacement){
            return "[" + *address.absolute + " + " + ::toString(*address.displacement) + "]";
        }

        if(address.base_register){
            return "[" + *address.absolute + " + " + to_string(*address.base_register) + "]";
        }

        return "[" + *address.absolute + "]";
    }
        
    if(address.base_register){
        if(address.scaled_register){
            if(address.scale){
                if(address.displacement){
                    return "[" + to_string(*address.base_register) + " + " + to_string(*address.scaled_register) + " * " + ::toString(*address.scale) + " + " + ::toString(*address.displacement) + "]";
                }
                
                return "[" + to_string(*address.base_register) + " + " + to_string(*address.scaled_register) + " * " + ::toString(*address.scale) + "]";
            }
            
            return "[" + to_string(*address.base_register) + " + " + to_string(*address.scaled_register) + "]";
        }

        return "[" + to_string(*address.base_register) + "]";
    }

    if(address.displacement){
        return "[" + ::toString(*address.displacement) + "]";
    }

    ASSERT_PATH_NOT_TAKEN("Invalid address type");
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
    
} //end of x86_64 namespace

using namespace x86_64;

std::ostream& operator<<(std::ostream& os, eddic::ltac::Argument& arg){
    if(auto* ptr = boost::get<int>(&arg)){
        return os << *ptr;
    } else if(auto* ptr = boost::get<double>(&arg)){
        return os << *ptr;
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

namespace eddic { namespace as {

struct X86_64StatementCompiler : public boost::static_visitor<> {
    AssemblyFileWriter& writer;

    X86_64StatementCompiler(AssemblyFileWriter& writer) : writer(writer) {
        //Nothing else to init
    }

    void operator()(std::shared_ptr<ltac::Instruction> instruction){
        switch(instruction->op){
            case ltac::Operator::MOV:
                if(boost::get<ltac::Address>(&*instruction->arg1)){
                    writer.stream() << "mov qword " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                } else {
                    writer.stream() << "mov " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
                }
                break;
            case ltac::Operator::FMOV:
                writer.stream() << "movsd " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
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
                writer.stream() << "push " << *instruction->arg1 << std::endl;
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
                writer.stream() << "imul " << *instruction->arg1 << ", " << *instruction->arg2 << ", " << *instruction->arg3 << std::endl;
                break;
            case ltac::Operator::DIV:
                writer.stream() << "div " << *instruction->arg1 << ", " << *instruction->arg2 << std::endl;
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

}} //end of eddic::as

namespace { //anonymous namespace

void saveFloat64(AssemblyFileWriter& writer, const std::vector<std::string>& registers){
    for(auto& reg : registers){
        writer.stream() << "sub rsp, 8" << std::endl;
        writer.stream() << "movq [rsp], " << reg << std::endl;
    }
}

void restoreFloat64(AssemblyFileWriter& writer, const std::vector<std::string>& registers){
    auto it = registers.rbegin();
    auto end = registers.rend();

    while(it != end){
        writer.stream() << "movq " << *it << ", [rsp]" << std::endl;
        writer.stream() << "add rsp, 8" << std::endl;
        ++it;
    }
}

void addPrintIntegerBody(AssemblyFileWriter& writer){
    //The parameter is in r14
    writer.stream() << "mov rax, r14" << std::endl;//We move it to rax for rax is the register source division register
    writer.stream() << "xor r14, r14" << std::endl;//We use r14 to be the counter (start with 0)

    //If the number is negative, we print the - and then the number
    writer.stream() << "or rax, rax" << std::endl;
    writer.stream() << "jge .loop" << std::endl;

    writer.stream() << "neg rax" << std::endl;

    //Print "-" 
    writer.stream() << "push S2" << std::endl;
    writer.stream() << "push 1" << std::endl;
    writer.stream() << "call _F5printS" << std::endl;
    writer.stream() << "add rsp, 16" << std::endl;

    //Divide rax until there is nothing to divide
    writer.stream() << ".loop" << ":" << std::endl;
    writer.stream() << "xor rdx, rdx" << std::endl;
    writer.stream() << "mov rbx, 10" << std::endl;
    writer.stream() << "div rbx" << std::endl;
    writer.stream() << "add rdx, 48" << std::endl;
    writer.stream() << "push rdx" << std::endl;
    writer.stream() << "inc r14" << std::endl;
    writer.stream() << "or rax, rax" << std::endl;
    writer.stream() << "jz .next" << std::endl;
    writer.stream() << "jmp .loop" << std::endl;

    //Print each of the char, one by one
    writer.stream() << ".next" << ":" << std::endl;
    writer.stream() << "or r14, r14" << std::endl;
    writer.stream() << "jz .exit" << std::endl;
    writer.stream() << "dec r14" << std::endl;

    writer.stream() << "mov rax, 1" << std::endl;       //syscall 1 = write
    writer.stream() << "mov rdi, 1" << std::endl;       //stdout
    writer.stream() << "mov rsi, rsp" << std::endl;     //read from the stack
    writer.stream() << "mov rdx, 1" << std::endl;       //length
    writer.stream() << "syscall" << std::endl;          //syscall

    writer.stream() << "add rsp, 8" << std::endl;

    writer.stream() << "jmp .next" << std::endl;

    writer.stream() << ".exit" << ":" << std::endl;
}

void addPrintIntegerFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printI");

    as::save(writer, {"rax", "rbx", "rcx", "rdx", "rsi", "rdi"});

    addPrintIntegerBody(writer);

    as::restore(writer, {"rax", "rbx", "rcx", "rdx", "rsi", "rdi"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnI");

    as::save(writer, {"rax", "rbx", "rcx", "rdx", "rsi", "rdi"});

    addPrintIntegerBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    as::restore(writer, {"rax", "rbx", "rcx", "rdx", "rsi", "rdi"});

    leaveFunction(writer);
}

void addPrintFloatBody(AssemblyFileWriter& writer){
    writer.stream() << "cvttsd2si rbx, xmm7" << std::endl;      //rbx = integer part
    writer.stream() << "cvtsi2sd xmm1, rbx" << std::endl;       //xmm1 = integer part

    //Print the integer part
    writer.stream() << "mov r14, rbx" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;

    //Print the dot char
    writer.stream() << "push S4" << std::endl;
    writer.stream() << "push 1" << std::endl;
    writer.stream() << "call _F5printS" << std::endl;
    writer.stream() << "add rsp, 16" << std::endl;

    //Handle negative numbers
    writer.stream() << "or rbx, rbx" << std::endl;
    writer.stream() << "jge .pos" << std::endl;
    writer.stream() << "mov rbx, __float64__(-1.0)" << std::endl;
    writer.stream() << "movq xmm2, rbx" << std::endl;
    writer.stream() << "mulsd xmm7, xmm2" << std::endl;
    writer.stream() << "mulsd xmm1, xmm2" << std::endl;

    writer.stream() << ".pos:" << std::endl;
   
    //Remove the integer part from the floating point 
    writer.stream() << "subsd xmm7, xmm1" << std::endl;         //xmm7 = decimal part
    
    writer.stream() << "mov rcx, __float64__(10000.0)" << std::endl;
    writer.stream() << "movq xmm2, rcx" << std::endl;           //xmm2 = 10'000
    
    writer.stream() << "mulsd xmm7, xmm2" << std::endl;         //xmm7 = decimal part * 10'000
    writer.stream() << "cvttsd2si rbx, xmm7" << std::endl;      //rbx = decimal part * 10'000
    writer.stream() << "mov rax, rbx" << std::endl;             //rax = rbx

    //Handle numbers with no decimal part 
    writer.stream() << "or rax, rax" << std::endl;
    writer.stream() << "je .end" << std::endl;
    
    //Handle numbers with 0 at the beginning of the decimal part
    writer.stream() << "xor r14, r14" << std::endl;
    writer.stream() << ".start:" << std::endl;
    writer.stream() << "cmp rax, 1000" << std::endl;
    writer.stream() << "jge .end" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << "imul rax, 10" << std::endl;
    writer.stream() << "jmp .start" << std::endl;
    
    //Print the number itself
    writer.stream() << ".end:" << std::endl;
    writer.stream() << "mov r14, rbx" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
}

void addPrintFloatFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printF");

    as::save(writer, {"rax", "rbx", "r14"});
    saveFloat64(writer, {"xmm1", "xmm2"});

    addPrintFloatBody(writer);

    restoreFloat64(writer, {"xmm1", "xmm2"});
    as::restore(writer, {"rax", "rbx", "r14"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnF");

    as::save(writer, {"rax", "rbx", "r14"});
    saveFloat64(writer, {"xmm1", "xmm2"});

    addPrintFloatBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    restoreFloat64(writer, {"xmm1", "xmm2"});
    as::restore(writer, {"rax", "rbx", "r14"});

    leaveFunction(writer);
}

void addPrintBoolBody(AssemblyFileWriter& writer){
    writer.stream() << "mov rax, [rbp + 16] " << std::endl;
    writer.stream() << "or rax, rax" << std::endl;
    writer.stream() << "jne .true_print" << std::endl;
    writer.stream() << "xor r14, r14" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << "jmp .end" << std::endl;
    writer.stream() << ".true_print:" << std::endl;
    writer.stream() << "mov r14, 1" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << ".end:" << std::endl;
}

void addPrintBoolFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printB");

    as::save(writer, {"rax"});

    addPrintBoolBody(writer);

    as::restore(writer, {"rax"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnB");

    as::save(writer, {"rax"});

    addPrintBoolBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    as::restore(writer, {"rax"});

    leaveFunction(writer);
}

void addPrintLineFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F7println");

    writer.stream() << "push S1" << std::endl;
    writer.stream() << "push 1" << std::endl;
    writer.stream() << "call _F5printS" << std::endl;
    writer.stream() << "add rsp, 16" << std::endl;

    leaveFunction(writer);
}

void addPrintStringBody(AssemblyFileWriter& writer){
    writer.stream() << "mov rax, 1" << std::endl;           //syscall 1 = write
    writer.stream() << "mov rdi, 1" << std::endl;           //stdout
    writer.stream() << "mov rsi, [rbp + 24]" << std::endl;  //length
    writer.stream() << "mov rdx, [rbp + 16]" << std::endl;  //source
    writer.stream() << "syscall" << std::endl;
}

void addPrintStringFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printS");
    
    as::save(writer, {"rax", "rcx", "rdi", "rsi", "rdx"});

    addPrintStringBody(writer);

    as::restore(writer, {"rax", "rcx", "rdi", "rsi", "rdx"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnS");
    
    as::save(writer, {"rax", "rcx", "rdi", "rsi", "rdx"});

    addPrintStringBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    as::restore(writer, {"rax", "rcx", "rdi", "rsi", "rdx"});

    leaveFunction(writer);
}

void addConcatFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "concat");

    writer.stream() << "mov rbx, [rbp + 32]" << std::endl;
    writer.stream() << "mov rcx, [rbp + 16]" << std::endl;
    writer.stream() << "add rbx, rcx" << std::endl;             //rbx = number of bytes = return 2

    //alloc the total number of bytes
    writer.stream() << "push rbx" << std::endl;
    writer.stream() << "call eddi_alloc" << std::endl;
    writer.stream() << "add rsp, 8" << std::endl;

    writer.stream() << "mov rdi, rax" << std::endl;             //destination address for the movsb
    
    writer.stream() << "mov rcx, [rbp + 32]" << std::endl;      //number of bytes of the source
    writer.stream() << "mov rsi, [rbp + 40]" << std::endl;      //source address

    writer.stream() << "rep movsb" << std::endl;                //copy the first part of the string into the destination

    writer.stream() << "mov rcx, [rbp + 16]" << std::endl;      //number of bytes of the source
    writer.stream() << "mov rsi, [rbp + 24]" << std::endl;      //source address

    writer.stream() << "rep movsb" << std::endl;                //copy the second part of the string into the destination

    leaveFunction(writer);
}

void addAllocFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "eddi_alloc");

    as::save(writer, {"rbx", "rcx", "rdx", "rdi", "rsi"});

    writer.stream() << "mov rcx, [rbp + 16]" << std::endl;
    writer.stream() << "mov rbx, [Veddi_remaining]" << std::endl;

    writer.stream() << "cmp rcx, rbx" << std::endl;
    writer.stream() << "jle .alloc_normal" << std::endl;

    //Get the current address
    writer.stream() << "mov rax, 12" << std::endl;          //syscall 12 = sys_brk
    writer.stream() << "xor rdi, rdi" << std::endl;         //get end
    writer.stream() << "syscall" << std::endl;

    //%eax is the current address 
    writer.stream() << "mov rsi, rax" << std::endl;

    //Alloc new block of 16384K from the current address
    writer.stream() << "mov rdi, rax" << std::endl;
    writer.stream() << "add rdi, 16384" << std::endl;       //rdi = first parameter
    writer.stream() << "mov rax, 12" << std::endl;          //syscall 12 = sys_brk
    writer.stream() << "syscall" << std::endl;

    //zero'd the new block
    writer.stream() << "mov rdi, rax" << std::endl;         //edi = start of block

    writer.stream() << "sub rdi, 4" << std::endl;           //edi points to the last DWORD available to us
    writer.stream() << "mov rcx, 4096" << std::endl;        //this many DWORDs were allocated
    writer.stream() << "xor rax, rax"  << std::endl;        //will write with zeroes
    writer.stream() << "std"  << std::endl;                 //walk backwards
    writer.stream() << "rep stosb"  << std::endl;           //write all over the reserved area
    writer.stream() << "cld"  << std::endl;                 //bring back the DF flag to normal state

    writer.stream() << "mov rax, rsi" << std::endl;

    //We now have 16K of available memory starting at %esi
    writer.stream() << "mov dword [Veddi_remaining], 16384" << std::endl;
    writer.stream() << "mov [Veddi_current], rsi" << std::endl;

    writer.stream() << ".alloc_normal:" << std::endl;

    //old = current
    writer.stream() << "mov rax, [Veddi_current]" << std::endl;

    //current += size
    writer.stream() << "mov rbx, [Veddi_current]" << std::endl;
    writer.stream() << "add rbx, rcx" << std::endl;
    writer.stream() << "mov [Veddi_current], rbx" << std::endl;

    //remaining -= size
    writer.stream() << "mov rbx, [Veddi_remaining]" << std::endl;
    writer.stream() << "sub rbx, rcx" << std::endl;
    writer.stream() << "mov [Veddi_remaining], rbx" << std::endl;

    writer.stream() << ".alloc_end:" << std::endl;

    as::restore(writer, {"rbx", "rcx", "rdx", "rdi", "rsi"});

    leaveFunction(writer);
}

void addTimeFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F4timeAI");

    writer.stream() << "xor rax, rax" << std::endl;
    writer.stream() << "cpuid" << std::endl;                //only to serialize instruction stream
    writer.stream() << "rdtsc" << std::endl;                //edx:eax = timestamp

    writer.stream() << "mov rsi, [rbp + 16]" << std::endl;
    writer.stream() << "mov [rsi - 4], eax" << std::endl;
    writer.stream() << "mov [rsi - 8], edx" << std::endl;

    leaveFunction(writer);
}

void addDurationFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F8durationAIAI");

    writer.stream() << "mov rsi, [rbp + 24]" << std::endl;          //Start time stamp
    writer.stream() << "mov rdi, [rbp + 16]" << std::endl;           //End time stamp

    //Print the high order bytes
    writer.stream() << "mov rax, [rsi - 8]" << std::endl;
    writer.stream() << "mov rbx, [rdi - 8]" << std::endl;
    writer.stream() << "sub rax, rbx" << std::endl;
   
    //if the first diff is 0, do not print 0
    writer.stream() << "cmp rax, 0" << std::endl;
    writer.stream() << "jz .second" << std::endl;

    //If it's negative, we print the positive only 
    writer.stream() << "cmp rax, 0" << std::endl;
    writer.stream() << "jge .push_first" << std::endl;
    writer.stream() << "neg rax" << std::endl;
    
    writer.stream() << ".push_first:" << std::endl; 
    writer.stream() << "push rax" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << "add rsp, 8" << std::endl;

    //Print the low order bytes
    writer.stream() << ".second:" << std::endl;
    writer.stream() << "mov rax, [rsi - 4]" << std::endl;
    writer.stream() << "mov rbx, [rdi - 4]" << std::endl;
    writer.stream() << "sub rax, rbx" << std::endl;
   
    //If it's negative, we print the positive only 
    writer.stream() << "cmp rax, 0" << std::endl;
    writer.stream() << "jge .push_second" << std::endl;
    writer.stream() << "neg rax" << std::endl;
   
    writer.stream() << ".push_second:" << std::endl; 
    writer.stream() << "push rax" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << "add rsp, 8" << std::endl;

    leaveFunction(writer);
}

} //end of anonymous namespace

void as::IntelX86_64CodeGenerator::addStandardFunctions(){
   addPrintIntegerFunction(writer); 
   addPrintBoolFunction(writer);
   addPrintFloatFunction(writer);
   addPrintLineFunction(writer); 
   addPrintStringFunction(writer); 
   addConcatFunction(writer);
   addAllocFunction(writer);
   addTimeFunction(writer);
   addDurationFunction(writer);
}
