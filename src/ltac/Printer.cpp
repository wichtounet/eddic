//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <memory>

#include <boost/variant.hpp>

#include "ltac/Printer.hpp"

#include "assert.hpp"
#include "VisitorUtils.hpp"
#include "Utils.hpp"

using namespace eddic;

namespace {

std::string to_string(ltac::Operator op){
    switch(op){
        case ltac::Operator::MOV:
            return "MOV"; 
        case ltac::Operator::FMOV:
            return "FMOV"; 
        case ltac::Operator::MEMSET:
            return "MEMSET"; 
        case ltac::Operator::ALLOC_STACK:
            return "ALLOC_STACK"; 
        case ltac::Operator::FREE_STACK:
            return "FREE_STACK"; 
        case ltac::Operator::LEAVE:
            return "LEAVE"; 
        case ltac::Operator::CMP_INT:
            return "CMP_INT"; 
        case ltac::Operator::CMP_FLOAT:
            return "CMP_FLOAT"; 
        case ltac::Operator::OR:
            return "OR"; 
        case ltac::Operator::XOR:
            return "XOR"; 
        case ltac::Operator::PUSH:
            return "PUSH"; 
        case ltac::Operator::POP:
            return "POP"; 
        case ltac::Operator::LEA:
            return "LEA"; 
        case ltac::Operator::SHIFT_LEFT:
            return "SHIFT_LEFT"; 
        case ltac::Operator::SHIFT_RIGHT:
            return "SHIFT_RIGHT"; 
        case ltac::Operator::ADD:
            return "ADD"; 
        case ltac::Operator::SUB:
            return "SUB"; 
        case ltac::Operator::MUL:
            return "MUL"; 
        case ltac::Operator::DIV:
            return "DIV"; 
        case ltac::Operator::FADD:
            return "FADD"; 
        case ltac::Operator::FSUB:
            return "FSUB"; 
        case ltac::Operator::FMUL:
            return "FMUL"; 
        case ltac::Operator::FDIV:
            return "FDIV"; 
        case ltac::Operator::INC:
            return "INC"; 
        case ltac::Operator::DEC:
            return "DEC"; 
        case ltac::Operator::NEG:
            return "NEG"; 
        case ltac::Operator::I2F:
            return "I2F"; 
        case ltac::Operator::F2I:
            return "F2I"; 
        case ltac::Operator::CMOVE:
            return "CMOVE"; 
        case ltac::Operator::CMOVNE:
            return "CMOVNE"; 
        case ltac::Operator::CMOVA:
            return "CMOVA"; 
        case ltac::Operator::CMOVAE:
            return "CMOVAE"; 
        case ltac::Operator::CMOVB:
            return "CMOVB"; 
        case ltac::Operator::CMOVBE:
            return "CMOVBE"; 
        case ltac::Operator::CMOVG:
            return "CMOVG"; 
        case ltac::Operator::CMOVGE:
            return "CMOVGE"; 
        case ltac::Operator::CMOVL:
            return "CMOVL"; 
        case ltac::Operator::CMOVLE:
            return "CMOVLE"; 
        default:
            ASSERT_PATH_NOT_TAKEN("The instruction operator is not supported");
    }
}

std::string to_string(ltac::JumpType type){
    switch(type){
        case ltac::JumpType::CALL:
            return "call";
        case ltac::JumpType::ALWAYS:
            return "always";
        case ltac::JumpType::NE:
            return "ne";
        case ltac::JumpType::E:
            return "e";
        case ltac::JumpType::GE:
            return "ge";
        case ltac::JumpType::G:
            return "g";
        case ltac::JumpType::LE:
            return "le";
        case ltac::JumpType::L:
            return "l";
        case ltac::JumpType::AE:
            return "ae";
        case ltac::JumpType::A:
            return "a";
        case ltac::JumpType::BE:
            return "be";
        case ltac::JumpType::B:
            return "b";
        case ltac::JumpType::P:
            return "p";
        case ltac::JumpType::Z:
            return "z";
        case ltac::JumpType::NZ:
            return "nz";
        default:
            ASSERT_PATH_NOT_TAKEN("The jump type is not supported");
    }
}

std::string printArg(ltac::Argument arg);

struct ArgumentToString : public boost::static_visitor<std::string> {
   std::string operator()(int& integer) const {
        return toString(integer);
   }
   
   std::string operator()(double& float_) const {
        return toString(float_);
   }

   std::string operator()(ltac::FloatRegister& reg) const {
        return "fr" + ::toString(reg.reg);
   }
   
   std::string operator()(ltac::Register& reg) const {
        return "ir" + ::toString(reg.reg);
   }
   
   std::string operator()(ltac::Address& address) const {
    if(address.absolute){
        if(address.displacement){
            return "[" + *address.absolute + " + " + toString(*address.displacement) + "]";
        }

        if(address.base_register){
            return "[" + *address.absolute + " + " + printArg(*address.base_register) + "]";
        }

        return "[" + *address.absolute + "]";
    }
        
    if(address.base_register){
        if(address.scaled_register){
            if(address.scale){
                if(address.displacement){
                    return "[" + printArg(*address.base_register) + " + " + printArg(*address.scaled_register) + " * " + ::toString(*address.scale) + " + " + ::toString(*address.displacement) + "]";
                }
                
                return "[" + printArg(*address.base_register) + " + " + printArg(*address.scaled_register) + " * " + ::toString(*address.scale) + "]";
            }
                
            if(address.displacement){
                return "[" + printArg(*address.base_register) + " + " + printArg(*address.scaled_register) + " + " + ::toString(*address.displacement) + "]";
            }
            
            return "[" + printArg(*address.base_register) + " + " + printArg(*address.scaled_register) + "]";
        }
        
        if(address.displacement){
            return "[" + printArg(*address.base_register) + " + " + ::toString(*address.displacement) + "]";
        }

        return "[" + printArg(*address.base_register) + "]";
    }

    if(address.displacement){
        return "[" + ::toString(*address.displacement) + "]";
    }

    ASSERT_PATH_NOT_TAKEN("Invalid address type");
   }
   
   std::string operator()(std::string& str) const {
        return str;
   }
};

std::string printArg(ltac::Argument arg){
    return visit(ArgumentToString(), arg);
}

struct DebugVisitor : public boost::static_visitor<> {
    void operator()(ltac::Program& program){
        std::cout << "LTAC Program " << std::endl << std::endl; 

        visit_each_non_variant(*this, program.functions);
    }

    void operator()(std::shared_ptr<ltac::Function>& function){
        std::cout << "Function " << function->getName() << std::endl;

        visit_each(*this, function->getStatements());

        std::cout << std::endl;
    }

    void operator()(std::shared_ptr<ltac::BasicBlock>& block){
        //Nothing to do here
        //TODO Can be removed in the future
    }

    void operator()(ltac::Statement& statement){
        visit(*this, statement);
    }

    void operator()(std::shared_ptr<ltac::Instruction>& quadruple){
        if(quadruple->arg1 && quadruple->arg2 && quadruple->arg3){
            std::cout << "\t" << to_string(quadruple->op) << " " << printArg(*quadruple->arg1) << ", " << printArg(*quadruple->arg2) << ", " << printArg(*quadruple->arg3) << std::endl;
        } else if(quadruple->arg1 && quadruple->arg2){
            std::cout << "\t" << to_string(quadruple->op) << " " << printArg(*quadruple->arg1) << ", " << printArg(*quadruple->arg2) << std::endl;
        } else if(quadruple->arg1){
            std::cout << "\t" << to_string(quadruple->op) << " " << printArg(*quadruple->arg1) << std::endl;
        } else {
            std::cout << "\t" << to_string(quadruple->op) << std::endl;
        }
    }

    void operator()(std::shared_ptr<ltac::Jump>& jmp){
        std::cout << "\tjmp (" << to_string(jmp->type) << ") " << jmp->label << std::endl;
    }

    void operator()(std::string& label){
        std::cout << "\t" << label << ":" << std::endl;
    }
};

} //end of anonymous namespace

void ltac::Printer::print(ltac::Program& program) const {
   DebugVisitor visitor;
   visitor(program); 
}
