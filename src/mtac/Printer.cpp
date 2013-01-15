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

struct ArgumentToString : public boost::static_visitor<std::string> {
   std::string operator()(const std::shared_ptr<Variable>& variable) const {
        std::string type = "";

        if(variable->type()->is_pointer()){
            type = "p";
        } else if(variable->type()->is_custom_type()){
            type = "c";
        } else if(variable->type()->is_standard_type()){
            if(variable->type() == FLOAT){
                type = "F";
            } else if(variable->type() == INT){
                type = "I";
            } else if(variable->type() == STRING){
                type = "S";
            } else if(variable->type() == CHAR){
                type = "C";
            } else if(variable->type() == BOOL){
                type = "B";
            } else {
                type = "s";
            }
        } else if(variable->type()->is_array()){
            type = "a";
        } else if(variable->type()->is_template_type()){
            type = "t";
        } else {
            type = "u";
        }

        if(variable->is_reference()){
            return variable->name() + "(ref," + type + ")";
        }

        switch(variable->position().type()){
            case PositionType::STACK:
                return variable->name() + "(s," + type + ")";
            case PositionType::PARAMETER:
                return variable->name() + "(p," + type + ")";
            case PositionType::GLOBAL:
                return variable->name() + "(g," + type + ")";
            case PositionType::CONST:
                return variable->name() + "(c," + type + ")";
            case PositionType::TEMPORARY:
                return variable->name() + "(t," + type + ")";
            case PositionType::VARIABLE:
                return variable->name() + "(v," + type + ")";
            case PositionType::REGISTER:
                return variable->name() + "(r," + type + ")";
            case PositionType::PARAM_REGISTER:
                return variable->name() + "(pr," + type + ")";
            default:
                eddic_unreachable("Unhandled position type");
        }
   }

   std::string operator()(const int& integer) const {
        return toString(integer);
   }
   
   std::string operator()(const double& float_) const {
        return toString(float_);
   }

   std::string operator()(const std::string& str) const {
        return str;
   }
};

std::string printArg(const mtac::Argument& arg){
    return visit(ArgumentToString(), arg);
}

std::string printVar(std::shared_ptr<Variable> var){
    return ArgumentToString()(var);
}

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

        //TODO Use a switch

        if(op == mtac::Operator::ASSIGN){
            stream << "\t" << printVar(quadruple.result) << " = (normal) " << printArg(*quadruple.arg1) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::FASSIGN){
            stream << "\t" << printVar(quadruple.result) << " = (float) " << printArg(*quadruple.arg1) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::PASSIGN){
            stream << "\t" << printVar(quadruple.result) << " = (pointer) " << printArg(*quadruple.arg1) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::ADD){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " + " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::FADD){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " + (float) " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::SUB){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " - " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::FSUB){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " - (float) " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::MUL){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " * " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::FMUL){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " * (float) " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::DIV){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " / " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::FDIV){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " / (float) " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::MOD){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " % " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::AND){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " & " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::EQUALS || op == mtac::Operator::FE){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " == " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::NOT_EQUALS || op == mtac::Operator::FNE){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " != " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::GREATER || op == mtac::Operator::FG){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " > " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::GREATER_EQUALS || op == mtac::Operator::FGE){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " >= " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::LESS || op == mtac::Operator::FL){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " < " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::LESS_EQUALS || op == mtac::Operator::FLE){
            stream << "\t" << printVar(quadruple.result) << " = " << printArg(*quadruple.arg1) << " <= " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::MINUS){
            stream << "\t" << printVar(quadruple.result) << " = - " << printArg(*quadruple.arg1) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::NOT){
            stream << "\t" << printVar(quadruple.result) << " = ! " << printArg(*quadruple.arg1) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::I2F){
            stream << "\t" << printVar(quadruple.result) << " = (cast float) " << printArg(*quadruple.arg1) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::F2I){
            stream << "\t" << printVar(quadruple.result) << " = (cast int) " << printArg(*quadruple.arg1) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::DOT){
            stream << "\t" << printVar(quadruple.result) << " = (normal) (" << printArg(*quadruple.arg1) << ")" << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::FDOT){
            stream << "\t" << printVar(quadruple.result) << " = (float) (" << printArg(*quadruple.arg1) << ")" << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::PDOT){
            stream << "\t" << printVar(quadruple.result) << " = (pointer) (" << printArg(*quadruple.arg1) << ")" << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::DOT_ASSIGN){
            stream << "\t" << "(" << printVar(quadruple.result) << ")" << printArg(*quadruple.arg1) << " = (normal) " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::DOT_FASSIGN){
            stream << "\t" << "(" << printVar(quadruple.result) << ")" << printArg(*quadruple.arg1) << " = (float) " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::DOT_PASSIGN){
            stream << "\t" << "(" << printVar(quadruple.result) << ")" << printArg(*quadruple.arg1) << " = (pointer) " << printArg(*quadruple.arg2) << " : "<< quadruple.depth << endl;
        } else if(op == mtac::Operator::RETURN){
            stream << "\t" << "return";

            if(quadruple.arg1){
                stream << " " << printArg(*quadruple.arg1);
            }

            if(quadruple.arg2){
                stream << ", " << printArg(*quadruple.arg2);
            }

            stream << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::NOP){
            stream << "\t" << "nop" << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::LABEL){
            stream << "\t" << quadruple.label() << ":" << endl;
        } else if(op == mtac::Operator::GOTO){
            if(quadruple.block){
                stream << "\t " << "goto " << "B" + toString(quadruple.block->index) << " : " << quadruple.depth << endl;
            } else {
                stream << "\t" << "goto " << quadruple.label() << " : " << quadruple.depth << endl;
            }
        } else if(op == mtac::Operator::PARAM){
            std::string address;
            if(quadruple.address){
                address = " address ";
            }

            if(quadruple.param()){
                stream << "\t" << "param " << address << "(" << printVar(quadruple.param()) << ") " << printArg(*quadruple.arg1) << " : " << quadruple.depth << endl;
            } else {
                if(quadruple.std_param().length() > 0){
                    stream << "\t" << "param " << address << "(std::" << quadruple.std_param() << ") " << printArg(*quadruple.arg1) << " : " << quadruple.depth << endl;
                } else {
                    stream << "\t" << "param " << address << printArg(*quadruple.arg1) << " : " << quadruple.depth << endl;
                }
            }
        } else if(op == mtac::Operator::CALL){
            stream << "\t";

            if(quadruple.return1()){
                stream << printVar(quadruple.return1());
            }

            if(quadruple.return2()){
                stream << ", " << printVar(quadruple.return2());
            }

            if(quadruple.return1() || quadruple.return2()){
                stream << " = ";
            }

            stream << "call " << quadruple.function().mangled_name() << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_FALSE_UNARY){
            stream << "\t" << "if_false " << printArg(*quadruple.arg1) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_FALSE_EQUALS || op == mtac::Operator::IF_FALSE_FE){
            stream << "\t" << "if_false " << printArg(*quadruple.arg1) << " == " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_FALSE_NOT_EQUALS || op == mtac::Operator::IF_FALSE_FNE){
            stream << "\t" << "if_false " << printArg(*quadruple.arg1) << " != " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_FALSE_LESS || op == mtac::Operator::IF_FALSE_FL){
            stream << "\t" << "if_false " << printArg(*quadruple.arg1) << " < " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_FALSE_LESS_EQUALS || op == mtac::Operator::IF_FALSE_FLE){
            stream << "\t" << "if_false " << printArg(*quadruple.arg1) << " <= " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_FALSE_GREATER || op == mtac::Operator::IF_FALSE_FG){
            stream << "\t" << "if_false " << printArg(*quadruple.arg1) << " > " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_FALSE_GREATER_EQUALS || op == mtac::Operator::IF_FALSE_FGE){
            stream << "\t" << "if_false " << printArg(*quadruple.arg1) << " >= " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_UNARY){
            stream << "\t" << "if " << printArg(*quadruple.arg1) << " goto " << printTarget(quadruple) << endl;
        } else if(op == mtac::Operator::IF_EQUALS || op == mtac::Operator::IF_FE){
            stream << "\t" << "if " << printArg(*quadruple.arg1) << " == " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_NOT_EQUALS || op == mtac::Operator::IF_FNE){
            stream << "\t" << "if " << printArg(*quadruple.arg1) << " != " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_LESS || op == mtac::Operator::IF_FL){
            stream << "\t" << "if " << printArg(*quadruple.arg1) << " < " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_LESS_EQUALS || op == mtac::Operator::IF_FLE){
            stream << "\t" << "if " << printArg(*quadruple.arg1) << " <= " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_GREATER || op == mtac::Operator::IF_FG){
            stream << "\t" << "if " << printArg(*quadruple.arg1) << " > " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
        } else if(op == mtac::Operator::IF_GREATER_EQUALS || op == mtac::Operator::IF_FGE){
            stream << "\t" << "if " << printArg(*quadruple.arg1) << " >= " << printArg(*quadruple.arg2) << " goto " << printTarget(quadruple) << " : " << quadruple.depth << endl;
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

void mtac::Printer::printStatement(const mtac::Quadruple& statement) const {
   DebugVisitor visitor;
   visitor.print(statement);
}

void mtac::Printer::printArgument(const mtac::Argument& arg) const {
    std::cout << printArg(arg) << std::endl;
}

void mtac::print(mtac::Program& program){
    mtac::Printer printer;
    printer.print(program);
}

void mtac::print(mtac::Function& function){
    mtac::Printer printer;
    printer.printFunction(function);
}
