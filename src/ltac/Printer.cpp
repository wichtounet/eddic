//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <iomanip>
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
        case ltac::Operator::XORPS:
            return "XORPS"; 
        case ltac::Operator::MOVDQU:
            return "MOVDQU"; 
        case ltac::Operator::CALL:
            return "call";
        case ltac::Operator::ALWAYS:
            return "always";
        case ltac::Operator::NE:
            return "ne";
        case ltac::Operator::E:
            return "e";
        case ltac::Operator::GE:
            return "ge";
        case ltac::Operator::G:
            return "g";
        case ltac::Operator::LE:
            return "le";
        case ltac::Operator::L:
            return "l";
        case ltac::Operator::AE:
            return "ae";
        case ltac::Operator::A:
            return "a";
        case ltac::Operator::BE:
            return "be";
        case ltac::Operator::B:
            return "b";
        case ltac::Operator::P:
            return "p";
        case ltac::Operator::Z:
            return "z";
        case ltac::Operator::NZ:
            return "nz";
        default:
            eddic_unreachable("The instruction operator is not supported");
    }
}

struct DebugVisitor : public boost::static_visitor<> {
    std::ostream& out;

    DebugVisitor(std::ostream& out) : out(out) {}

    void operator()(mtac::Program& program){
        out << "LTAC Program " << std::endl << std::endl; 

        visit_each_non_variant(*this, program.functions);
    }

    void operator()(mtac::Function& function){
        out << "Function " << function.get_name() << std::endl;

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
        out << "\t" << std::setw(3) << std::setfill('0') << quadruple->uid() << ": ";

        if(quadruple->is_jump()){
            out << "jmp (" << to_string(quadruple->op) << ") " << quadruple->label << std::endl;
        } else {
            out << to_string(quadruple->op);

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

void ltac::Printer::print(mtac::Program& program) const {
   DebugVisitor visitor(std::cout);
   visitor(program); 
}

void ltac::Printer::print(mtac::Function& function) const {
   DebugVisitor visitor(std::cout);
   visitor(function); 
}

void ltac::Printer::print(ltac::Statement& statement) const {
   DebugVisitor visitor(std::cout);
   visit(visitor, statement); 
}
