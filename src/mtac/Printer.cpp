//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <memory>

#include <boost/variant.hpp>

#include "mtac/Printer.hpp"
#include "mtac/Program.hpp"

#include "VisitorUtils.hpp"
#include "Utils.hpp"

using namespace eddic;

namespace {

struct ArgumentToString : public boost::static_visitor<std::string> {
   std::string operator()(std::shared_ptr<Variable>& variable) const {
        switch(variable->position().type()){
            case PositionType::STACK:
                return variable->name() + "(s)";
            case PositionType::PARAMETER:
                return variable->name() + "(p)";
            case PositionType::GLOBAL:
                return variable->name() + "(g)";
            case PositionType::CONST:
                return variable->name() + "(c)";
            case PositionType::TEMPORARY:
                return variable->name() + "(t)";
            case PositionType::PARAM_REGISTER:
                return variable->name() + "(pr)";
        }

        return variable->name();   
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

struct DebugVisitor : public boost::static_visitor<> {
    void operator()(std::shared_ptr<mtac::Program> program){
        std::cout << "TAC Program " << std::endl << std::endl; 

        visit_each_non_variant(*this, program->functions);
    }

    void operator()(std::shared_ptr<mtac::Function>& function){
        std::cout << "Function " << function->getName() << std::endl;

        visit_each(*this, function->getStatements());
        visit_each_non_variant(*this, function->getBasicBlocks());

        std::cout << std::endl;
    }

    void operator()(std::shared_ptr<mtac::BasicBlock>& block){
        if(block->index == -1){
            std::cout << "ENTRY" << std::endl;
        } else if(block->index == -2){
            std::cout << "EXIT" << std::endl;
        } else {
            std::cout << "B" << block->index << "->" << std::endl;
            visit_each(*this, block->statements);     
        }
    }

    void operator()(mtac::Statement& statement){
        visit(*this, statement);
    }

    void operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
        auto op = quadruple->op;

        if(op == mtac::Operator::ASSIGN){
            std::cout << "\t" << quadruple->result->name() << " = (normal) " << printArg(*quadruple->arg1) << std::endl;
        } else if(op == mtac::Operator::FASSIGN){
            std::cout << "\t" << quadruple->result->name() << " = (float) " << printArg(*quadruple->arg1) << std::endl;
        } else if(op == mtac::Operator::PASSIGN){
            std::cout << "\t" << quadruple->result->name() << " = (pointer) " << printArg(*quadruple->arg1) << std::endl;
        } else if(op == mtac::Operator::ADD){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " + " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::FADD){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " + (float) " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::SUB){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " - " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::FSUB){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " - (float) " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::MUL){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " * " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::FMUL){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " * (float) " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::DIV){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " / " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::FDIV){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " / (float) " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::MOD){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " % " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::EQUALS || op == mtac::Operator::FE){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " == " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::NOT_EQUALS || op == mtac::Operator::FNE){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " != " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::GREATER || op == mtac::Operator::FG){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " > " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::GREATER_EQUALS || op == mtac::Operator::FGE){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " >= " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::LESS || op == mtac::Operator::FL){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " < " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::LESS_EQUALS || op == mtac::Operator::FLE){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " <= " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::MINUS){
            std::cout << "\t" << quadruple->result->name() << " = - " << printArg(*quadruple->arg1) << std::endl;
        } else if(op == mtac::Operator::I2F){
            std::cout << "\t" << quadruple->result->name() << " = (cast float) " << printArg(*quadruple->arg1) << std::endl;
        } else if(op == mtac::Operator::F2I){
            std::cout << "\t" << quadruple->result->name() << " = (cast int) " << printArg(*quadruple->arg1) << std::endl;
        } else if(op == mtac::Operator::DOT){
            std::cout << "\t" << quadruple->result->name() << " = (normal) (" << printArg(*quadruple->arg1) << ")" << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::FDOT){
            std::cout << "\t" << quadruple->result->name() << " = (float) (" << printArg(*quadruple->arg1) << ")" << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::DOT_ASSIGN){
            std::cout << "\t(" << quadruple->result->name() << ")" << printArg(*quadruple->arg1) << " = (normal) " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::DOT_FASSIGN){
            std::cout << "\t(" << quadruple->result->name() << ")" << printArg(*quadruple->arg1) << " = (float) " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::ARRAY){
            std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " [" << printArg(*quadruple->arg2) << "]" << std::endl;
        } else if(op == mtac::Operator::ARRAY_ASSIGN){
            std::cout << "\t" << quadruple->result->name() << "[" << printArg(*quadruple->arg1) << "] = " << printArg(*quadruple->arg2) << std::endl;
        } else if(op == mtac::Operator::RETURN){
            std::cout << "\treturn";

            if(quadruple->arg1){
                std::cout << " " << printArg(*quadruple->arg1);
            }

            if(quadruple->arg2){
                std::cout << ", " << printArg(*quadruple->arg2);
            }

            std::cout << std::endl;
        } else if(op == mtac::Operator::NOP){
            std::cout << "\tnop" << std::endl;
        }
    }

    template<typename T>
    std::string printTarget(std::shared_ptr<T>& ifFalse){
        if(ifFalse->block){
            return "B" + toString(ifFalse->block->index);   
        } else {
            return ifFalse->label;
        }
    }

    void operator()(std::shared_ptr<mtac::IfFalse>& ifFalse){
        if(ifFalse->op){
            auto op = *ifFalse->op;
            if(op == mtac::BinaryOperator::EQUALS || op == mtac::BinaryOperator::FE){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " == " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::NOT_EQUALS || op == mtac::BinaryOperator::FNE){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " != " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::LESS || op == mtac::BinaryOperator::FL){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " < " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::LESS_EQUALS || op == mtac::BinaryOperator::FLE){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " <= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::GREATER || op == mtac::BinaryOperator::FG){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " > " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::GREATER_EQUALS || op == mtac::BinaryOperator::FGE){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " >= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            }
        } else {
            std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " goto " << printTarget(ifFalse) << std::endl;
        }
    }

    void operator()(std::shared_ptr<mtac::If>& ifFalse){
        if(ifFalse->op){
            auto op = *ifFalse->op;
            if(op == mtac::BinaryOperator::EQUALS || op == mtac::BinaryOperator::FE){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " == " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::NOT_EQUALS || op == mtac::BinaryOperator::FNE){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " != " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::LESS || op == mtac::BinaryOperator::FL){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " < " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::LESS_EQUALS || op == mtac::BinaryOperator::FLE){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " <= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::GREATER || op == mtac::BinaryOperator::FG){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " > " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == mtac::BinaryOperator::GREATER_EQUALS || op == mtac::BinaryOperator::FGE){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " >= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            }
        } else {
            std::cout << "\tif " << printArg(ifFalse->arg1) << " goto " << printTarget(ifFalse) << std::endl;
        }
    }
    
    void operator()(std::shared_ptr<mtac::Param>& param){
        std::string address;
        if(param->address){
            address = " address ";
        }

        std::string members;
        for(std::size_t i = 0; i < param->memberNames.size(); ++i){
            members += "." + param->memberNames[i];
        }

        if(param->param){
            std::cout << "\tparam " << address << "(" << param->param->name() << ") " << printArg(param->arg) << members << std::endl;
        } else {
            if(param->std_param.length() > 0){
                std::cout << "\tparam " << address << "(std::" << param->std_param << ") " << printArg(param->arg) << members << std::endl;
            } else {
                std::cout << "\tparam " << address << printArg(param->arg) << members << std::endl;
            }
        }
    }

    void operator()(std::shared_ptr<mtac::Goto>& goto_){
        std::cout << "\tgoto " << printTarget(goto_) << std::endl;
    }

    void operator()(std::shared_ptr<mtac::NoOp>&){
        std::cout << "\tno-op" << std::endl;
    }

    void operator()(std::shared_ptr<mtac::Call>& call){
        std::cout << "\t";

        if(call->return_){
            std::cout << call->return_->name();
        }

        if(call->return2_){
            std::cout << ", " << call->return2_->name();
        }

        if(call->return_ || call->return2_){
            std::cout << " = ";
        }

        std::cout << "call " << call->function << std::endl;
    }

    void operator()(std::string& label){
        std::cout << "\t" << label << ":" << std::endl;
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

void mtac::Printer::printStatement(mtac::Statement& statement) const {
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
