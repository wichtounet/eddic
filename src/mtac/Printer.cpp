//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <memory>

#include "variant.hpp"
#include "assert.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"

#include "mtac/Printer.hpp"
#include "mtac/Program.hpp"

using namespace eddic;

namespace {

struct ArgumentToString : public boost::static_visitor<std::string> {
   std::string operator()(std::shared_ptr<Variable> variable) const {
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
        } else if(variable->type()->is_template()){
            type = "t";
        } else {
            type = "u";
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
                ASSERT_PATH_NOT_TAKEN("Unhandled position type");
        }
   }

   std::string operator()(int& integer) const {
        return toString(integer);
   }
   
   std::string operator()(double& float_) const {
        return toString(float_);
   }

   std::string operator()(std::string& str) const {
        return str;
   }
};

std::string printArg(mtac::Argument& arg){
    return visit(ArgumentToString(), arg);
}

std::string printVar(std::shared_ptr<Variable> var){
    return ArgumentToString()(var);
}

typedef std::ostream& (*manipulator)(std::ostream&); 

struct DebugVisitor : public boost::static_visitor<> {
    manipulator endl = std::endl;
    std::ostream& stream;

    DebugVisitor() : stream(std::cout) {}
    DebugVisitor(std::ostream& os) : stream(os) {}

    void operator()(std::shared_ptr<mtac::Program> program){
        stream << "TAC Program " << endl << endl; 

        visit_each_non_variant(*this, program->functions);
    }

    void operator()(std::shared_ptr<mtac::Function> function){
        stream << "Function " << function->getName() << endl;

        visit_each(*this, function->getStatements());

        for(auto& block : function){
            visit_non_variant(*this, block);
        }

        stream << endl;
    }

    void pretty_print(std::vector<std::shared_ptr<mtac::BasicBlock>> blocks){
        if(blocks.empty()){
            stream << "{}";
        } else {
            stream << "{" << blocks[0];

            for(std::size_t i = 1; i < blocks.size(); ++i){
                stream << ", " << blocks[i];
            }

            stream << "}";
        }
    }

    void operator()(std::shared_ptr<mtac::BasicBlock> block){
        std::string sep(25, '-');

        stream << sep << std::endl;
        stream << block;

        stream << " prev: " << block->prev << ", next: " << block->next << std::endl;
        stream << "successors "; pretty_print(block->successors); std::cout << std::endl;;
        stream << "predecessors "; pretty_print(block->predecessors); std::cout << std::endl;;
        
        stream << sep << std::endl;
        
        visit_each(*this, block->statements);     
        
    }

    void operator()(mtac::Statement& statement){
        visit(*this, statement);
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        auto op = quadruple->op;

        if(op == mtac::Operator::ASSIGN){
            stream << "\t" << printVar(quadruple->result) << " = (normal) " << printArg(*quadruple->arg1) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::FASSIGN){
            stream << "\t" << printVar(quadruple->result) << " = (float) " << printArg(*quadruple->arg1) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::PASSIGN){
            stream << "\t" << printVar(quadruple->result) << " = (pointer) " << printArg(*quadruple->arg1) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::ADD){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " + " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::FADD){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " + (float) " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::SUB){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " - " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::FSUB){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " - (float) " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::MUL){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " * " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::FMUL){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " * (float) " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::DIV){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " / " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::FDIV){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " / (float) " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::MOD){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " % " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::AND){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " & " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::EQUALS || op == mtac::Operator::FE){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " == " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::NOT_EQUALS || op == mtac::Operator::FNE){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " != " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::GREATER || op == mtac::Operator::FG){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " > " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::GREATER_EQUALS || op == mtac::Operator::FGE){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " >= " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::LESS || op == mtac::Operator::FL){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " < " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::LESS_EQUALS || op == mtac::Operator::FLE){
            stream << "\t" << printVar(quadruple->result) << " = " << printArg(*quadruple->arg1) << " <= " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::MINUS){
            stream << "\t" << printVar(quadruple->result) << " = - " << printArg(*quadruple->arg1) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::NOT){
            stream << "\t" << printVar(quadruple->result) << " = ! " << printArg(*quadruple->arg1) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::I2F){
            stream << "\t" << printVar(quadruple->result) << " = (cast float) " << printArg(*quadruple->arg1) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::F2I){
            stream << "\t" << printVar(quadruple->result) << " = (cast int) " << printArg(*quadruple->arg1) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::DOT){
            stream << "\t" << printVar(quadruple->result) << " = (normal) (" << printArg(*quadruple->arg1) << ")" << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::FDOT){
            stream << "\t" << printVar(quadruple->result) << " = (float) (" << printArg(*quadruple->arg1) << ")" << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::PDOT){
            stream << "\t" << printVar(quadruple->result) << " = (pointer) (" << printArg(*quadruple->arg1) << ")" << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::DOT_ASSIGN){
            stream << "\t(" << printVar(quadruple->result) << ")" << printArg(*quadruple->arg1) << " = (normal) " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::DOT_FASSIGN){
            stream << "\t(" << printVar(quadruple->result) << ")" << printArg(*quadruple->arg1) << " = (float) " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::DOT_PASSIGN){
            stream << "\t(" << printVar(quadruple->result) << ")" << printArg(*quadruple->arg1) << " = (pointer) " << printArg(*quadruple->arg2) << " : "<< quadruple->depth << endl;
        } else if(op == mtac::Operator::RETURN){
            stream << "\treturn";

            if(quadruple->arg1){
                stream << " " << printArg(*quadruple->arg1);
            }

            if(quadruple->arg2){
                stream << ", " << printArg(*quadruple->arg2);
            }

            stream << " : " << quadruple->depth << endl;
        } else if(op == mtac::Operator::NOP){
            stream << "\tnop" << " : " << quadruple->depth << endl;
        }
    }

    template<typename T>
    std::string printTarget(std::shared_ptr<T> ifFalse){
        if(ifFalse->block){
            return "B" + toString(ifFalse->block->index);   
        } else {
            return ifFalse->label;
        }
    }

    void operator()(std::shared_ptr<mtac::IfFalse> ifFalse){
        if(ifFalse->op){
            auto op = *ifFalse->op;
            if(op == mtac::BinaryOperator::EQUALS || op == mtac::BinaryOperator::FE){
                stream << "\tifFalse " << printArg(ifFalse->arg1) << " == " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::NOT_EQUALS || op == mtac::BinaryOperator::FNE){
                stream << "\tifFalse " << printArg(ifFalse->arg1) << " != " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::LESS || op == mtac::BinaryOperator::FL){
                stream << "\tifFalse " << printArg(ifFalse->arg1) << " < " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::LESS_EQUALS || op == mtac::BinaryOperator::FLE){
                stream << "\tifFalse " << printArg(ifFalse->arg1) << " <= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::GREATER || op == mtac::BinaryOperator::FG){
                stream << "\tifFalse " << printArg(ifFalse->arg1) << " > " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::GREATER_EQUALS || op == mtac::BinaryOperator::FGE){
                stream << "\tifFalse " << printArg(ifFalse->arg1) << " >= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            }
        } else {
            stream << "\tifFalse " << printArg(ifFalse->arg1) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
        }
    }

    void operator()(std::shared_ptr<mtac::If> ifFalse){
        if(ifFalse->op){
            auto op = *ifFalse->op;
            if(op == mtac::BinaryOperator::EQUALS || op == mtac::BinaryOperator::FE){
                stream << "\tif " << printArg(ifFalse->arg1) << " == " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::NOT_EQUALS || op == mtac::BinaryOperator::FNE){
                stream << "\tif " << printArg(ifFalse->arg1) << " != " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::LESS || op == mtac::BinaryOperator::FL){
                stream << "\tif " << printArg(ifFalse->arg1) << " < " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::LESS_EQUALS || op == mtac::BinaryOperator::FLE){
                stream << "\tif " << printArg(ifFalse->arg1) << " <= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::GREATER || op == mtac::BinaryOperator::FG){
                stream << "\tif " << printArg(ifFalse->arg1) << " > " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            } else if(op == mtac::BinaryOperator::GREATER_EQUALS || op == mtac::BinaryOperator::FGE){
                stream << "\tif " << printArg(ifFalse->arg1) << " >= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << " : " << ifFalse->depth << endl;
            }
        } else {
            stream << "\tif " << printArg(ifFalse->arg1) << " goto " << printTarget(ifFalse) << endl;
        }
    }
    
    void operator()(std::shared_ptr<mtac::Param> param){
        std::string address;
        if(param->address){
            address = " address ";
        }

        std::string members;

        if(param->param){
            stream << "\tparam " << address << "(" << printVar(param->param) << ") " << printArg(param->arg) << " : " << param->depth << endl;
        } else {
            if(param->std_param.length() > 0){
                stream << "\tparam " << address << "(std::" << param->std_param << ") " << printArg(param->arg) << " : " << param->depth << endl;
            } else {
                stream << "\tparam " << address << printArg(param->arg) << " : " << param->depth << endl;
            }
        }
    }

    void operator()(std::shared_ptr<mtac::Goto> goto_){
        stream << "\tgoto " << printTarget(goto_) << " : " << goto_->depth << endl;
    }

    void operator()(std::shared_ptr<mtac::NoOp>){
        stream << "\tno-op" << endl;
    }

    void operator()(std::shared_ptr<mtac::Call> call){
        stream << "\t";

        if(call->return_){
            stream << printVar(call->return_);
        }

        if(call->return2_){
            stream << ", " << printVar(call->return2_);
        }

        if(call->return_ || call->return2_){
            stream << " = ";
        }

        stream << "call " << call->function << " : " << call->depth << endl;
    }

    void operator()(std::string& label){
        stream << "\t" << label << ":" << endl;
    }
};

} //end of anonymous namespace

void mtac::Printer::print(std::shared_ptr<mtac::Program> program) const {
   DebugVisitor visitor;
   visitor(program); 
}

void mtac::Printer::printFunction(std::shared_ptr<mtac::Function> function) const {
   DebugVisitor visitor;
   visitor(function); 
}

std::ostream& inline_manipulator(std::ostream& os){
    return os;
}

void mtac::Printer::print_inline(mtac::Statement statement, std::ostream& os) const {
   DebugVisitor visitor(os);
   visitor.endl = inline_manipulator;
   visit(visitor, statement);
}

void mtac::Printer::printStatement(mtac::Statement statement) const {
   DebugVisitor visitor;
   visit(visitor, statement);
}

void mtac::Printer::printArgument(mtac::Argument& arg) const {
    std::cout << printArg(arg) << std::endl;
}

void mtac::print(std::shared_ptr<mtac::Program> program){
    mtac::Printer printer;
    printer.print(program);
}

void mtac::print(std::shared_ptr<mtac::Function> function){
    mtac::Printer printer;
    printer.printFunction(function);
}
