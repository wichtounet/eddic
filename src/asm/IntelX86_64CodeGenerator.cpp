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

namespace x86_64 {

enum Register {
    RAX,
    RBX,
    RCX,
    RDX,

    R8, 
    R9, 
    R10,
    R11, 
    R12,
    R13,
    R14,
    R15,

    RSP, //Extended stack pointer
    RBP, //Extended base pointer

    RSI, //Extended source index
    RDI, //Extended destination index
    
    REGISTER_COUNT  
};

std::string regToString(Register reg){
    static std::string registers[Register::REGISTER_COUNT] = {
        "rax", "rbx", "rcx", "rdx", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
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
    
} //end of x86_64 namespace

using namespace x86_64;

namespace eddic { namespace as {

struct IntelX86_64StatementCompiler : public IntelStatementCompiler<Register>, public boost::static_visitor<> {
    IntelX86_64StatementCompiler(AssemblyFileWriter& w, std::shared_ptr<tac::Function> f) : IntelStatementCompiler(w, {RDI, RSI, RCX, RDX, R8, R9, R10, R11, R12, R13, R14, R15, RBX, RAX}, f) {}
    
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
        writer.stream() << "sar rdx, 63" << std::endl;

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

}} //end of eddic::as

namespace { //anonymous namespace

void compile(AssemblyFileWriter& writer, std::shared_ptr<tac::BasicBlock> block, as::IntelX86_64StatementCompiler& compiler){
    compiler.reset();

    if(compiler.blockUsage.find(block) != compiler.blockUsage.end()){
        writer.stream() << block->label << ":" << std::endl;
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
 
} //end of anonymous space

namespace eddic { namespace as {

void IntelX86_64CodeGenerator::compile(std::shared_ptr<tac::Function> function){
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
    for(auto block : function->getBasicBlocks()){
        block->label = newLabel();
    }

    //Then we compile each of them
    for(auto block : function->getBasicBlocks()){
        ::compile(writer, block, compiler);
    }
 
    if(function->getBasicBlocks().size() > 0){
        auto& lastBasicBlock = function->getBasicBlocks().back();
        
        if(lastBasicBlock->statements.size() > 0){
            auto lastStatement = lastBasicBlock->statements.back();
            
            if(auto* ptr = boost::get<std::shared_ptr<tac::Quadruple>>(&lastStatement)){
                if((*ptr)->op && *(*ptr)->op != tac::Operator::RETURN){
                    //Only if necessary, deallocates size on the stack for the local variables
                    if(size > 0){
                        writer.stream() << "add rsp, " << size << std::endl;
                    }

                    leaveFunction(writer);

                    return;
                }
            }
        }
    }
                    
    //Only if necessary, deallocates size on the stack for the local variables
    if(size > 0){
        writer.stream() << "add rsp, " << size << std::endl;
    }

    leaveFunction(writer);
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
        writer.stream() << "add rsp, 8" << std::endl;

        writer.stream() << "lea rsi, [rax + rcx - 8]" << std::endl;         //esi = last address of the array
        writer.stream() << "mov rdx, rsi" << std::endl;                     //edx = last address of the array
        
        writer.stream() << "mov [rsi], rbx" << std::endl;                   //Set the length of the array
        writer.stream() << "sub rsi, 8" << std::endl;                       //Move to the destination address of the first arg

        writer.stream() << ".copy_args:" << std::endl;
        writer.stream() << "pop rdi" << std::endl;                          //edi = address of current args
        writer.stream() << "mov [rsi+8], rdi" << std::endl;                 //set the address of the string

        /* Calculate the length of the string  */
        writer.stream() << "xor rax, rax" << std::endl;
        writer.stream() << "xor rcx, rcx" << std::endl;
        writer.stream() << "not rcx" << std::endl;
        writer.stream() << "repne scasb" << std::endl;
        writer.stream() << "not rcx" << std::endl;
        writer.stream() << "dec rcx" << std::endl;
        /* End of the calculation */

        writer.stream() << "mov qword [rsi], rcx" << std::endl;               //set the length of the string
        writer.stream() << "sub rsi, 8" << std::endl;
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

}} //end of eddic::as

namespace { //anonymous namespace

void addPrintIntegerBody(AssemblyFileWriter& writer){
    writer.stream() << "mov rax, [rbp+16]" << std::endl;
    writer.stream() << "xor r8, r8" << std::endl;

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
    writer.stream() << "inc r8" << std::endl;
    writer.stream() << "cmp rax, 0" << std::endl;
    writer.stream() << "jz .next" << std::endl;
    writer.stream() << "jmp .loop" << std::endl;

    //Print each of the char, one by one
    writer.stream() << ".next" << ":" << std::endl;
    writer.stream() << "cmp r8, 0" << std::endl;
    writer.stream() << "jz .exit" << std::endl;
    writer.stream() << "dec r8" << std::endl;

    writer.stream() << "mov rax, 1" << std::endl;       //syscall 1 = write
    writer.stream() << "mov rdi, 1" << std::endl;       //stdout
    writer.stream() << "mov rsi, rsp" << std::endl;     //read from the stack
    writer.stream() << "mov rdx, 1" << std::endl;       //length
    writer.stream() << "syscall" << std::endl;          //syscall

    writer.stream() << "add rsp, 8" << std::endl;

    writer.stream() << "jmp .next" << std::endl;

    writer.stream() << ".exit" << ":" << std::endl;
}

void save(AssemblyFileWriter& writer, const std::vector<std::string>& registers){
    for(auto& reg : registers){
        writer.stream() << "push " << reg << std::endl;
    }
}

void restore(AssemblyFileWriter& writer, const std::vector<std::string>& registers){
    auto it = registers.rbegin();
    auto end = registers.rend();

    while(it != end){
        writer.stream() << "pop " << *it << std::endl;
        ++it;
    }
}

void addPrintIntegerFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printI");

    save(writer, {"rax", "rbx", "rdx", "rsi", "rdi", "r8"});

    addPrintIntegerBody(writer);

    restore(writer, {"rax", "rbx", "rdx", "rsi", "rdi", "r8"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnI");

    save(writer, {"rax", "rbx", "rdx", "rsi", "rdi", "r8"});

    addPrintIntegerBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    restore(writer, {"rax", "rbx", "rdx", "rsi", "rdi", "r8"});

    leaveFunction(writer);
}

void addPrintBoolBody(AssemblyFileWriter& writer){
    writer.stream() << "mov rax, [rbp + 16] " << std::endl;
    writer.stream() << "or rax, rax" << std::endl;
    writer.stream() << "jne .true_print" << std::endl;
    writer.stream() << "push 0" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << "jmp .end" << std::endl;
    writer.stream() << ".true_print:" << std::endl;
    writer.stream() << "push 1" << std::endl;
    writer.stream() << "call _F5printI" << std::endl;
    writer.stream() << "add rsp, 8" << std::endl;
    writer.stream() << ".end:" << std::endl;
}

void addPrintBoolFunction(AssemblyFileWriter& writer){
    defineFunction(writer, "_F5printB");

    save(writer, {"rax"});

    addPrintBoolBody(writer);

    restore(writer, {"rax"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnB");

    save(writer, {"rax"});

    addPrintBoolBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    restore(writer, {"rax"});

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
    
    save(writer, {"rax", "rdi", "rsi", "rdx"});

    addPrintStringBody(writer);

    restore(writer, {"rax", "rdi", "rsi", "rdx"});

    leaveFunction(writer);
   
    /* println version */
    
    defineFunction(writer, "_F7printlnS");
    
    save(writer, {"rax", "rdi", "rsi", "rdx"});

    addPrintStringBody(writer);

    writer.stream() << "call _F7println" << std::endl;

    restore(writer, {"rax", "rdi", "rsi", "rdx"});

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

    save(writer, {"rbx", "rcx", "rdx", "rdi", "rsi"});

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

    restore(writer, {"rbx", "rcx", "rdx", "rdi", "rsi"});

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
   addPrintLineFunction(writer); 
   addPrintStringFunction(writer); 
   addConcatFunction(writer);
   addAllocFunction(writer);
   addTimeFunction(writer);
   addDurationFunction(writer);
}