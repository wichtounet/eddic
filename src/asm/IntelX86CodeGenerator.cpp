//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <ostream>

#include "assert.hpp"
#include "AssemblyFileWriter.hpp"
#include "FunctionContext.hpp"
#include "Labels.hpp"
#include "VisitorUtils.hpp"
#include "GlobalContext.hpp"

#include "mtac/Program.hpp"

#include "asm/StringConverter.hpp"
#include "asm/IntelX86CodeGenerator.hpp"
#include "asm/IntelAssemblyUtils.hpp"

using namespace eddic;

as::IntelX86CodeGenerator::IntelX86CodeGenerator(AssemblyFileWriter& w, mtac::Program& program, std::shared_ptr<GlobalContext> context) : IntelCodeGenerator(w, program, context) {}

namespace {
    
const std::string registers[6] = {"eax", "ebx", "ecx", "edx", "esi", "edi"};
const std::string registers_8[6] = {"al", "bl", "cl", "dl", "", ""};
const std::string registers_16[6] = {"ax", "bx", "cx", "dx", "si", "di"};

const std::string float_registers[8] = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"};

struct X86_32StringConverter : public as::StringConverter, public boost::static_visitor<std::string> {
    std::string operator()(ltac::Register& reg) const {
        if(static_cast<int>(reg) == 1000){
            return "esp"; 
        } else if(static_cast<int>(reg) == 1001){
            return "ebp"; 
        }

        return registers[static_cast<int>(reg)];
    }
    
    std::string operator()(ltac::FloatRegister& reg) const {
        return float_registers[static_cast<int>(reg)];
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

    std::string operator()(ltac::PseudoRegister&) const {
        eddic_unreachable("All the pseudo registers should have been converted into a hard register");
    }

    std::string operator()(ltac::PseudoFloatRegister&) const {
        eddic_unreachable("All the pseudo registers should have been converted into a hard register");
    }

    std::string operator()(double value) const {
        std::stringstream ss;
        ss << "__float32__(" << std::fixed << value << ")";
        return ss.str();
    }
};

} //end of anonymous namespace

namespace x86 {

std::ostream& operator<<(std::ostream& os, eddic::ltac::Argument& arg){
    X86_32StringConverter converter;
    return os << visit(converter, arg);
}

} //end of x86 namespace

using namespace x86;

namespace {

std::string get_register_8(ltac::Register& reg){
    eddic_assert(reg.reg < 6, "SP and BP registers cannot be subclassed");
    auto sub_reg = registers_8[reg.reg];
    eddic_assert(!sub_reg.empty(), "RSI and RDI are not 8-bit allocatable");
    return sub_reg;
}

std::string get_register_16(ltac::Register& reg){
    eddic_assert(reg.reg < 6, "SP and BP registers cannot be subclassed");
    return registers_16[reg.reg];
}

void compile_statement(AssemblyFileWriter& writer, ltac::Instruction& instruction){
    switch(instruction.op){
        case ltac::Operator::LABEL:
            writer.stream() << "." << instruction.label << ":" << '\n';
            break;
        case ltac::Operator::MOV:
            if(instruction.size != ltac::Size::DEFAULT){
                if(boost::get<ltac::Address>(&*instruction.arg1)){
                    if(auto* ptr = boost::get<ltac::Register>(&*instruction.arg2)){
                        switch(instruction.size){
                            case ltac::Size::BYTE:
                                writer.stream() << "mov byte " << *instruction.arg1 << ", " << get_register_8(*ptr) << '\n';
                                break;
                            case ltac::Size::WORD:
                                writer.stream() << "mov word " << *instruction.arg1 << ", " << get_register_16(*ptr) << '\n';
                                break;
                            default:
                                writer.stream() << "mov dword " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
                                break;
                        }
                    } else {
                        switch(instruction.size){
                            case ltac::Size::BYTE:
                                writer.stream() << "mov byte " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
                                break;
                            case ltac::Size::WORD:
                                writer.stream() << "mov word " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
                                break;
                            default:
                                writer.stream() << "mov dword " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
                                break;
                        }
                    }
                } else {
                    //TODO The instruction should always be mov (and not movzx) to avoid having something context-dependent
                    //movzx should be chosen higher
                    switch(instruction.size){
                        case ltac::Size::BYTE:
                            writer.stream() << "movzx " << *instruction.arg1 << ", byte " << *instruction.arg2 << '\n';
                            break;
                        case ltac::Size::WORD:
                            writer.stream() << "movzx " << *instruction.arg1 << ", word " << *instruction.arg2 << '\n';
                            break;
                        default:
                            writer.stream() << "mov " << *instruction.arg1 << ", dword " << *instruction.arg2 << '\n';
                            break;
                    }
                }

                break;
            }

            if(boost::get<ltac::FloatRegister>(&*instruction.arg1) && boost::get<ltac::Register>(&*instruction.arg2)){
                writer.stream() << "movd " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            } else if(boost::get<ltac::Register>(&*instruction.arg1) && boost::get<ltac::FloatRegister>(&*instruction.arg2)){
                writer.stream() << "movd " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            } else if(boost::get<ltac::Address>(&*instruction.arg1)){
                writer.stream() << "mov dword " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            } else {
                writer.stream() << "mov " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            }

            break;
        case ltac::Operator::FMOV:
            if(boost::get<ltac::FloatRegister>(&*instruction.arg1) && boost::get<ltac::Register>(&*instruction.arg2)){
                writer.stream() << "movd " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            } else {
                writer.stream() << "movss " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            }

            break;
        case ltac::Operator::ENTER:
            writer.stream() << "push ebp" << '\n';
            writer.stream() << "mov ebp, esp" << '\n';
            break;
        case ltac::Operator::LEAVE:
            writer.stream() << "mov esp, ebp" << '\n';
            writer.stream() << "pop ebp" << '\n';
            break;
        case ltac::Operator::RET:
            writer.stream() << "ret" << '\n';
            break;
        case ltac::Operator::CMP_INT:
            writer.stream() << "cmp " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMP_FLOAT:
            writer.stream() << "ucomiss " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::OR:
            writer.stream() << "or " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::XOR:
            writer.stream() << "xor " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::PUSH:
            if(boost::get<ltac::Address>(&*instruction.arg1)){
                writer.stream() << "push dword " << *instruction.arg1 << '\n';
            } else {
                writer.stream() << "push " << *instruction.arg1 << '\n';
            }

            break;
        case ltac::Operator::POP:
            writer.stream() << "pop " << *instruction.arg1 << '\n';
            break;
        case ltac::Operator::LEA:
            writer.stream() << "lea " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::SHIFT_LEFT:
            writer.stream() << "sal " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::SHIFT_RIGHT:
            writer.stream() << "sar " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::ADD:
            writer.stream() << "add " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::SUB:
            writer.stream() << "sub " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::MUL2:
        case ltac::Operator::MUL3:
            if(instruction.arg3){
                writer.stream() << "imul " << *instruction.arg1 << ", " << *instruction.arg2 << ", " << *instruction.arg3 << '\n';
            } else {
                writer.stream() << "imul " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            }

            break;
        case ltac::Operator::DIV:
            writer.stream() << "idiv " << *instruction.arg1 << '\n';
            break;
        case ltac::Operator::FADD:
            writer.stream() << "addss " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::FSUB:
            writer.stream() << "subss " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::FMUL:
            writer.stream() << "mulss " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::FDIV:
            writer.stream() << "divss " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::INC:
            writer.stream() << "inc " << *instruction.arg1 << '\n';
            break;
        case ltac::Operator::DEC:
            writer.stream() << "dec " << *instruction.arg1 << '\n';
            break;
        case ltac::Operator::NEG:
            writer.stream() << "neg " << *instruction.arg1 << '\n';
            break;
        case ltac::Operator::NOT:
            writer.stream() << "not " << *instruction.arg1 << '\n';
            break;
        case ltac::Operator::AND:
            writer.stream() << "and " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::I2F:
            writer.stream() << "cvtsi2ss " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::F2I:
            writer.stream() << "cvttss2si " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVE:
            writer.stream() << "cmove " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVNE:
            writer.stream() << "cmovne " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVA:
            writer.stream() << "cmova " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVAE:
            writer.stream() << "cmovae " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVB:
            writer.stream() << "cmovb " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVBE:
            writer.stream() << "cmovbe " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVG:
            writer.stream() << "cmovg " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVGE:
            writer.stream() << "cmovge " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVL:
            writer.stream() << "cmovl " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::CMOVLE:
            writer.stream() << "cmovle " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::XORPS:
            writer.stream() << "xorps " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::MOVDQU:
            writer.stream() << "movdqu " << *instruction.arg1 << ", " << *instruction.arg2 << '\n';
            break;
        case ltac::Operator::NOP:
            //Nothing to output for a nop
            break;
        case ltac::Operator::CALL:
            writer.stream() << "call " << instruction.label << '\n';
            break;
        case ltac::Operator::ALWAYS:
            writer.stream() << "jmp " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::NE:
            writer.stream() << "jne " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::E:
            writer.stream() << "je " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::GE:
            writer.stream() << "jge " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::G:
            writer.stream() << "jg " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::LE:
            writer.stream() << "jle " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::L:
            writer.stream() << "jl " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::AE:
            writer.stream() << "jae " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::A:
            writer.stream() << "ja" << "." << instruction.label << '\n';
            break;
        case ltac::Operator::BE:
            writer.stream() << "jbe " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::B:
            writer.stream() << "jb " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::P:
            writer.stream() << "jp " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::Z:
            writer.stream() << "jz " << "." << instruction.label << '\n';
            break;
        case ltac::Operator::NZ:
            writer.stream() << "jnz " << "." << instruction.label << '\n';
            break;
        default:
            eddic_unreachable("The instruction operator is not supported");
    }
}

} //end of anonymous namespace

void as::IntelX86CodeGenerator::compile(mtac::Function& function){
    writer.stream() << '\n' << function.get_name() << ":" << '\n';

    for(auto& bb : function){
        for(auto& statement : bb->l_statements){
            compile_statement(writer, statement);
        }
    }
}

void as::IntelX86CodeGenerator::writeRuntimeSupport(){
    writer.stream() << "section .text" << '\n' << '\n';

    writer.stream() << "global _start" << '\n' << '\n';

    writer.stream() << "_start:" << '\n';
    
    //If necessary init memory manager 
    if(context->exists("_F4mainAS") 
            || program.call_graph.is_reachable(context->getFunction("_F4freePI")) 
            || program.call_graph.is_reachable(context->getFunction("_F5allocI"))){
        writer.stream() << "call _F4init" << '\n'; 
    }

    //If the user wants the args, we add support for them
    if(context->exists("_F4mainAS")){
        writer.stream() << "pop ebx" << '\n';                          //ebx = number of args
        
        writer.stream() << "lea ecx, [4 + ebx * 8]" << '\n';           //ecx = size of the array
        writer.stream() << "call _F5allocI" << '\n';                  //eax = start address of the array

        writer.stream() << "mov esi, eax" << '\n';         //esi = last address of the array
        writer.stream() << "mov edx, esi" << '\n';                     //edx = last address of the array
        
        writer.stream() << "mov [esi], ebx" << '\n';                   //Set the length of the array
        writer.stream() << "add esi, 4" << '\n';                       //Move to the destination address of the first arg

        writer.stream() << ".copy_args:" << '\n';
        writer.stream() << "pop edi" << '\n';                          //edi = address of current args
        writer.stream() << "mov [esi], edi" << '\n';                 //set the address of the string

        /* Calculate the length of the string  */
        writer.stream() << "xor eax, eax" << '\n';
        writer.stream() << "xor ecx, ecx" << '\n';
        writer.stream() << "not ecx" << '\n';
        writer.stream() << "repne scasb" << '\n';
        writer.stream() << "not ecx" << '\n';
        writer.stream() << "dec ecx" << '\n';
        /* End of the calculation */

        writer.stream() << "mov [esi+4], ecx" << '\n';               //set the length of the string
        writer.stream() << "add esi, 8" << '\n';
        writer.stream() << "dec ebx" << '\n';
        writer.stream() << "jnz .copy_args" << '\n';

        writer.stream() << "push edx" << '\n';
    }

    /* Give control to the user main function */
    if(context->exists("_F4mainAS")){
        writer.stream() << "call _F4mainAS" << '\n';
    } else {
        writer.stream() << "call _F4main" << '\n';
    }
    
    /* Exit the program */
    writer.stream() << "mov eax, 1" << '\n';
    writer.stream() << "xor ebx, ebx" << '\n';
    writer.stream() << "int 80h" << '\n';
}

void as::IntelX86CodeGenerator::defineDataSection(){
    writer.stream() << '\n' << "section .data" << '\n';
}

void as::IntelX86CodeGenerator::declareIntArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<'\n';
    writer.stream() << "dd " << size << '\n';
    writer.stream() << "times " << size << " dd 0" << '\n';
}

void as::IntelX86CodeGenerator::declareFloatArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<'\n';
    writer.stream() << "dd " << size << '\n';
    writer.stream() << "times " << size << " dd __float32__(0.0)" << '\n';
}

void as::IntelX86CodeGenerator::declareStringArray(const std::string& name, unsigned int size){
    writer.stream() << "V" << name << ":" <<'\n';
    writer.stream() << "dd " << size << '\n';
    writer.stream() << "%rep " << size << '\n';
    writer.stream() << "dd S1" << '\n';
    writer.stream() << "dd 0" << '\n';
    writer.stream() << "%endrep" << '\n';
}

void as::IntelX86CodeGenerator::declareIntVariable(const std::string& name, int value){
    writer.stream() << "V" << name << " dd " << value << '\n';
}

void as::IntelX86CodeGenerator::declareStringVariable(const std::string& name, const std::string& label, int size){
    writer.stream() << "V" << name << " dd " << label << ", " << size << '\n';
}

void as::IntelX86CodeGenerator::declareString(const std::string& label, const std::string& value){
    writer.stream() << label << " dd " << value << '\n';
}

void as::IntelX86CodeGenerator::declareFloat(const std::string& label, double value){
    writer.stream() << std::fixed << label << " dd __float32__(" << value << ")" << '\n';
}

void as::IntelX86CodeGenerator::addStandardFunctions(){
    if(program.call_graph.is_reachable(context->getFunction("_F5printC"))){
        output_function("x86_32_printC");
    }
    
    if(program.call_graph.is_reachable(context->getFunction("_F5printS"))){ 
        output_function("x86_32_printS");
    }
    
    //Memory management functions are included the three together
    if(context->exists("_F4mainAS") || program.call_graph.is_reachable(context->getFunction("_F4freePI")) || program.call_graph.is_reachable(context->getFunction("_F5allocI"))){
        output_function("x86_32_alloc");
        output_function("x86_32_init");
        output_function("x86_32_free");
    }
    
    if(program.call_graph.is_reachable(context->getFunction("_F4timeAI"))){
        output_function("x86_32_time");
    }
    
    if(program.call_graph.is_reachable(context->getFunction("_F8durationAIAI"))){
        output_function("x86_32_duration");
    }
    
    if(program.call_graph.is_reachable(context->getFunction("_F9read_char"))){
        output_function("x86_32_read_char");
    }
}
