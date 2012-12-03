//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <memory>

#include "assert.hpp"
#include "variant.hpp"
#include "VisitorUtils.hpp"
#include "Utils.hpp"

#include "mtac/Program.hpp"

#include "ltac/Printer.hpp"
#include "ltac/Statement.hpp"

using namespace eddic;

namespace {

std::string to_string(ltac::Operator op){
    switch(op){
        case ltac::Operator::MOV:
            return "MOV"; 
        case ltac::Operator::ENTER:
            return "ENTER"; 
        case ltac::Operator::FMOV:
            return "FMOV"; 
        case ltac::Operator::MEMSET:
            return "MEMSET"; 
        case ltac::Operator::LEAVE:
            return "LEAVE"; 
        case ltac::Operator::RET:
            return "RET"; 
        case ltac::Operator::PRE_RET:
            return "PRE_RET"; 
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
        case ltac::Operator::MUL2:
        case ltac::Operator::MUL3:
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
        case ltac::Operator::NOT:
            return "NOT"; 
        case ltac::Operator::AND:
            return "AND"; 
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
        case ltac::Operator::PRE_PARAM:
            return "PRE_PARAM"; 
        case ltac::Operator::NOP:
            return "NOP"; 
        default:
            eddic_unreachable("The instruction operator is not supported");
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
            eddic_unreachable("The jump type is not supported");
    }
}

struct DebugVisitor : public boost::static_visitor<> {
    std::ostream& out;

    DebugVisitor(std::ostream& out) : out(out) {}

    void operator()(mtac::program_p program){
        out << "LTAC Program " << std::endl << std::endl; 

        visit_each_non_variant(*this, program->functions);
    }

    void operator()(mtac::function_p function){
        out << "Function " << function->getName() << std::endl;

        for(auto& bb : function){
            visit_non_variant(*this, bb);
        }

        out << std::endl;
    }
    
    void operator()(mtac::basic_block_p bb){
        pretty_print(bb, out);

        visit_each(*this, bb->l_statements);
    }

    void operator()(const ltac::Statement& statement){
        visit(*this, statement);
    }

    void operator()(std::shared_ptr<ltac::Instruction> quadruple){
        out << "\t" << to_string(quadruple->op);

        if(quadruple->arg1){
            out << " " << *quadruple->arg1;
            
            if(quadruple->arg2){
                out << ", " << *quadruple->arg2;

                if(quadruple->arg3){
                    out << ", " << *quadruple->arg3;
                }
            }
        }

        out << std::endl;
    }

    void operator()(const std::shared_ptr<ltac::Jump> jmp){
        out << "\tjmp (" << to_string(jmp->type) << ") " << jmp->label << std::endl;
    }

    void operator()(const std::string& label){
        out << "\t" << label << ":" << std::endl;
    }
};

} //end of anonymous namespace

void ltac::print_statement(const ltac::Statement& statement, std::ostream& out){
   DebugVisitor visitor(out);
   visit(visitor, statement); 
}

void ltac::Printer::print(mtac::program_p program) const {
   DebugVisitor visitor(std::cout);
   visitor(program); 
}

void ltac::Printer::print(mtac::function_p function) const {
   DebugVisitor visitor(std::cout);
   visitor(function); 
}

void ltac::Printer::print(ltac::Statement& statement) const {
   DebugVisitor visitor(std::cout);
   visit(visitor, statement); 
}
