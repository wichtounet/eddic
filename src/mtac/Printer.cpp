//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <iomanip>
#include <memory>

#include "variant.hpp"
#include "assert.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"
#include "Variable.hpp"
#include "Function.hpp"

#include "mtac/Printer.hpp"
#include "mtac/Program.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

namespace {

typedef std::ostream& (*manipulator)(std::ostream&); 

struct DebugVisitor {
    manipulator endl = std::endl;
    std::ostream& stream;

    DebugVisitor() : stream(std::cout) {}
    DebugVisitor(std::ostream& os) : stream(os) {}

    void print(mtac::Program& program){
        stream << "TAC Program " << endl << endl; 

        for(auto& function : program.functions){
            print(function);
        }
    }

    void print(mtac::Function& function){
        stream << "Function " << function.get_name() << "(pure:" << function.pure() << ")" <<endl;

        for(auto& quadruple : function.get_statements()){
            print(quadruple);
        }

        for(auto& block : function){
            print(block);
        }

        stream << endl;
    }

    void print(mtac::basic_block_p block){
        pretty_print(block, stream);
        
        for(auto& quadruple : block->statements){
            print(quadruple);
        }
    }

    void print(const mtac::Quadruple& quadruple){
        auto op = quadruple.op;

        stream << "\t" << std::setw(3) << std::setfill('0') << quadruple.uid() << ":";

        switch(op){
            case mtac::Operator::ASSIGN:
                stream << "\t" << quadruple.result << " = (normal) " << *quadruple.arg1 << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::FASSIGN:
                stream << "\t" << quadruple.result << " = (float) " << *quadruple.arg1 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::PASSIGN:
                stream << "\t" << quadruple.result << " = (pointer) " << *quadruple.arg1 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::ADD:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " + " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::FADD:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " + (float) " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::SUB:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " - " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::FSUB:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " - (float) " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::MUL:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " * " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::FMUL:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " * (float) " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::DIV:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " / " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::FDIV:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " / (float) " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::MOD:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " % " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::AND:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " & " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::EQUALS:
            case mtac::Operator::FE:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " == " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::NOT_EQUALS:
            case mtac::Operator::FNE:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " != " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::GREATER:
            case mtac::Operator::FG:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " > " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::GREATER_EQUALS:
            case mtac::Operator::FGE:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " >= " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::LESS:
            case mtac::Operator::FL:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " < " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::LESS_EQUALS:
            case mtac::Operator::FLE:
                stream << "\t" << quadruple.result << " = " << *quadruple.arg1 << " <= " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::MINUS:
                stream << "\t" << quadruple.result << " = - " << *quadruple.arg1 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::FMINUS:
                stream << "\t" << quadruple.result << " = - (float) " << *quadruple.arg1 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::NOT:
                stream << "\t" << quadruple.result << " = ! " << *quadruple.arg1 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::I2F:
                stream << "\t" << quadruple.result << " = (cast float) " << *quadruple.arg1 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::F2I:
                stream << "\t" << quadruple.result << " = (cast int) " << *quadruple.arg1 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::DOT:
                stream << "\t" << quadruple.result << " = (normal) (" << *quadruple.arg1 << ")" << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::FDOT:
                stream << "\t" << quadruple.result << " = (float) (" << *quadruple.arg1 << ")" << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::PDOT:
                stream << "\t" << quadruple.result << " = (pointer) (" << *quadruple.arg1 << ")" << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::DOT_ASSIGN:
                stream << "\t" << "(" << quadruple.result << ")" << *quadruple.arg1 << " = (normal) " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::DOT_FASSIGN:
                stream << "\t" << "(" << quadruple.result << ")" << *quadruple.arg1 << " = (float) " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::DOT_PASSIGN:
                stream << "\t" << "(" << quadruple.result << ")" << *quadruple.arg1 << " = (pointer) " << *quadruple.arg2 << " : "<< quadruple.depth << endl;
                break;
            case mtac::Operator::RETURN:
                stream << "\t" << "return";

                if(quadruple.arg1){
                    stream << " " << *quadruple.arg1;
                }

                if(quadruple.arg2){
                    stream << ", " << *quadruple.arg2;
                }

                stream << " : " << quadruple.depth << endl;

                break;
            case  mtac::Operator::NOP:
                stream << "\t" << "nop" << " : " << quadruple.depth << endl;
                break;
            case  mtac::Operator::LABEL:
                stream << "\t" << quadruple.label() << ":" << endl;
                break;
            case  mtac::Operator::GOTO:
                if(quadruple.block){
                    stream << "\t " << "goto " << "B" + toString(quadruple.block->index) << " : " << quadruple.depth << endl;
                } else {
                    stream << "\t" << "goto " << quadruple.label() << " : " << quadruple.depth << endl;
                }

                break;
            case mtac::Operator::PARAM:
                if(quadruple.param()){
                    stream << "\t" << "param " << "(" << quadruple.param() << ") " << *quadruple.arg1 << " : " << quadruple.depth << endl;
                } else {
                    if(quadruple.std_param().length() > 0){
                        stream << "\t" << "param " << "(std::" << quadruple.std_param() << ") " << *quadruple.arg1 << " : " << quadruple.depth << endl;
                    } else {
                        stream << "\t" << "param " << *quadruple.arg1 << " : " << quadruple.depth << endl;
                    }
                }

                break;
            case  mtac::Operator::PPARAM:
                if(quadruple.param()){
                    stream << "\t" << "param address " << "(" << quadruple.param() << ") " << *quadruple.arg1 << " : " << quadruple.depth << endl;
                } else {
                    if(quadruple.std_param().length() > 0){
                        stream << "\t" << "param address " << "(std::" << quadruple.std_param() << ") " << *quadruple.arg1 << " : " << quadruple.depth << endl;
                    } else {
                        stream << "\t" << "param address " << *quadruple.arg1 << " : " << quadruple.depth << endl;
                    }
                }

                break;
            case  mtac::Operator::CALL:
                stream << "\t";

                if(quadruple.return1()){
                    stream << quadruple.return1();
                }

                if(quadruple.return2()){
                    stream << ", " << quadruple.return2();
                }

                if(quadruple.return1() || quadruple.return2()){
                    stream << " = ";
                }

                stream << "call " << quadruple.function().mangled_name() << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_FALSE_UNARY:
                stream << "\t" << "if_false " << *quadruple.arg1 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_FALSE_EQUALS:
            case mtac::Operator::IF_FALSE_FE:
                stream << "\t" << "if_false " << *quadruple.arg1 << " == " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_FALSE_NOT_EQUALS:
            case mtac::Operator::IF_FALSE_FNE:
                stream << "\t" << "if_false " << *quadruple.arg1 << " != " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_FALSE_LESS:
            case mtac::Operator::IF_FALSE_FL:
                stream << "\t" << "if_false " << *quadruple.arg1 << " < " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_FALSE_LESS_EQUALS:
            case mtac::Operator::IF_FALSE_FLE:
                stream << "\t" << "if_false " << *quadruple.arg1 << " <= " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_FALSE_GREATER:
            case mtac::Operator::IF_FALSE_FG:
                stream << "\t" << "if_false " << *quadruple.arg1 << " > " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_FALSE_GREATER_EQUALS:
            case mtac::Operator::IF_FALSE_FGE:
                stream << "\t" << "if_false " << *quadruple.arg1 << " >= " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_UNARY:
                stream << "\t" << "if " << *quadruple.arg1 << " goto " << printTarget(quadruple) << endl;
                break;
            case mtac::Operator::IF_EQUALS:
            case mtac::Operator::IF_FE:
                stream << "\t" << "if " << *quadruple.arg1 << " == " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_NOT_EQUALS:
            case mtac::Operator::IF_FNE:
                stream << "\t" << "if " << *quadruple.arg1 << " != " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_LESS:
            case mtac::Operator::IF_FL:
                stream << "\t" << "if " << *quadruple.arg1 << " < " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_LESS_EQUALS:
            case mtac::Operator::IF_FLE:
                stream << "\t" << "if " << *quadruple.arg1 << " <= " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_GREATER:
            case mtac::Operator::IF_FG:
                stream << "\t" << "if " << *quadruple.arg1 << " > " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
            case mtac::Operator::IF_GREATER_EQUALS:
            case mtac::Operator::IF_FGE:
                stream << "\t" << "if " << *quadruple.arg1 << " >= " << *quadruple.arg2 << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
                break;
        }
    }

    std::string printTarget(const mtac::Quadruple& quadruple){
        if(quadruple.block){
            return "B" + toString(quadruple.block->index);   
        } else {
            return quadruple.label();
        }
    }
};

} //end of anonymous namespace

void mtac::Printer::print(mtac::Program& program) const {
   DebugVisitor visitor;
   visitor.print(program);
}

void mtac::Printer::printFunction(mtac::Function& function) const {
   DebugVisitor visitor;
   visitor.print(function); 
}

std::ostream& inline_manipulator(std::ostream& os){
    return os;
}

void mtac::Printer::print_inline(const mtac::Quadruple& statement, std::ostream& os) const {
   DebugVisitor visitor(os);
   visitor.endl = inline_manipulator;
   visitor.print(statement);
}

void mtac::print(mtac::Program& program){
    mtac::Printer printer;
    printer.print(program);
}

void mtac::print(mtac::Function& function){
    mtac::Printer printer;
    printer.printFunction(function);
}
