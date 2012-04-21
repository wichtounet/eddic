//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <memory>

#include <boost/variant.hpp>

#include "tac/Printer.hpp"
#include "tac/Program.hpp"

#include "VisitorUtils.hpp"
#include "Utils.hpp"

using namespace eddic;

namespace {

struct ArgumentToString : public boost::static_visitor<std::string> {
   std::string operator()(std::shared_ptr<Variable>& variable) const {
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

std::string printArg(tac::Argument& arg){
    return visit(ArgumentToString(), arg);
}

struct DebugVisitor : public boost::static_visitor<> {
    void operator()(tac::Program& program){
        std::cout << "TAC Program " << std::endl << std::endl; 

        visit_each_non_variant(*this, program.functions);
    }

    void operator()(std::shared_ptr<tac::Function>& function){
        std::cout << "Function " << function->getName() << std::endl;

        visit_each(*this, function->getStatements());
        visit_each_non_variant(*this, function->getBasicBlocks());

        std::cout << std::endl;
    }

    void operator()(std::shared_ptr<tac::BasicBlock>& block){
        std::cout << "B" << block->index << "->" << std::endl;
        
        visit_each(*this, block->statements);     
    }

    void operator()(tac::Statement& statement){
        visit(*this, statement);
    }

    void operator()(std::shared_ptr<tac::Quadruple>& quadruple){
        if(quadruple->op == tac::Operator::ASSIGN){
            std::cout << "\t" << quadruple->result->name() << " = (normal) " << printArg(*quadruple->arg1) << std::endl;
        } else if(quadruple->op == tac::Operator::FASSIGN){
            std::cout << "\t" << quadruple->result->name() << " = (float) " << printArg(*quadruple->arg1) << std::endl;
        } else {
            tac::Operator op = quadruple->op;

            if(op == tac::Operator::ADD){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " + " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::FADD){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " + (float) " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::SUB){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " - " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::FSUB){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " - (float) " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::MUL){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " * " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::FMUL){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " * (float) " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::DIV){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " / " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::FDIV){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " / (float) " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::MOD){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " % " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::EQUALS || op == tac::Operator::FE){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " == " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::NOT_EQUALS || op == tac::Operator::FNE){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " != " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::GREATER || op == tac::Operator::FG){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " > " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::GREATER_EQUALS || op == tac::Operator::FGE){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " >= " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::LESS || op == tac::Operator::FL){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " < " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::LESS_EQUALS || op == tac::Operator::FLE){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " <= " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::MINUS){
                std::cout << "\t" << quadruple->result->name() << " = - " << printArg(*quadruple->arg1) << std::endl;
            } else if(op == tac::Operator::I2F){
                std::cout << "\t" << quadruple->result->name() << " = (cast float) " << printArg(*quadruple->arg1) << std::endl;
            } else if(op == tac::Operator::F2I){
                std::cout << "\t" << quadruple->result->name() << " = (cast int) " << printArg(*quadruple->arg1) << std::endl;
            } else if(op == tac::Operator::DOT){
                std::cout << "\t" << quadruple->result->name() << " = (" << printArg(*quadruple->arg1) << ")" << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::FDOT){
                std::cout << "\t" << quadruple->result->name() << " = (float) (" << printArg(*quadruple->arg1) << ")" << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::DOT_ASSIGN){
                std::cout << "\t(" << quadruple->result->name() << ")" << printArg(*quadruple->arg1) << " = " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::DOT_FASSIGN){
                std::cout << "\t(" << quadruple->result->name() << ")" << printArg(*quadruple->arg1) << " = (float) " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::ARRAY){
                std::cout << "\t" << quadruple->result->name() << " = " << printArg(*quadruple->arg1) << " [" << printArg(*quadruple->arg2) << "]" << std::endl;
            } else if(op == tac::Operator::ARRAY_ASSIGN){
                std::cout << "\t" << quadruple->result->name() << "[" << printArg(*quadruple->arg1) << "] = " << printArg(*quadruple->arg2) << std::endl;
            } else if(op == tac::Operator::RETURN){
                std::cout << "\treturn";

                if(quadruple->arg1){
                    std::cout << " " << printArg(*quadruple->arg1);
                }

                if(quadruple->arg2){
                    std::cout << ", " << printArg(*quadruple->arg2);
                }

                std::cout << std::endl;
            }
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

    void operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
        if(ifFalse->op){
            auto op = *ifFalse->op;
            if(op == tac::BinaryOperator::EQUALS || op == tac::BinaryOperator::FE){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " == " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::NOT_EQUALS || op == tac::BinaryOperator::FNE){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " != " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::LESS || op == tac::BinaryOperator::FL){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " < " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::LESS_EQUALS || op == tac::BinaryOperator::FLE){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " <= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::GREATER || op == tac::BinaryOperator::FG){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " > " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::GREATER_EQUALS || op == tac::BinaryOperator::FGE){
                std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " >= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            }
        } else {
            std::cout << "\tifFalse " << printArg(ifFalse->arg1) << " goto " << printTarget(ifFalse) << std::endl;
        }
    }

    void operator()(std::shared_ptr<tac::If>& ifFalse){
        if(ifFalse->op){
            auto op = *ifFalse->op;
            if(op == tac::BinaryOperator::EQUALS || op == tac::BinaryOperator::FE){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " == " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::NOT_EQUALS || op == tac::BinaryOperator::FNE){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " != " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::LESS || op == tac::BinaryOperator::FL){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " < " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::LESS_EQUALS || op == tac::BinaryOperator::FLE){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " <= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::GREATER || op == tac::BinaryOperator::FG){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " > " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            } else if(op == tac::BinaryOperator::GREATER_EQUALS || op == tac::BinaryOperator::FGE){
                std::cout << "\tif " << printArg(ifFalse->arg1) << " >= " << printArg(*ifFalse->arg2) << " goto " << printTarget(ifFalse) << std::endl;
            }
        } else {
            std::cout << "\tif " << printArg(ifFalse->arg1) << " goto " << printTarget(ifFalse) << std::endl;
        }
    }
    
    void operator()(std::shared_ptr<tac::Param>& param){
        if(param->param){
            std::cout << "\tparam (" << param->param->name() << ") " << printArg(param->arg) << std::endl;
        } else {
            if(param->std_param.length() > 0){
                std::cout << "\tparam (std::" << param->std_param << ") " << printArg(param->arg) << std::endl;
            } else {
                std::cout << "\tparam " << printArg(param->arg) << std::endl;
            }
        }
    }

    void operator()(std::shared_ptr<tac::Goto>& goto_){
        std::cout << "\tgoto " << printTarget(goto_) << std::endl;
    }

    void operator()(tac::NoOp&){
        std::cout << "\tno-op" << std::endl;
    }

    void operator()(std::shared_ptr<tac::Call>& call){
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

void tac::Printer::print(tac::Program& program) const {
   DebugVisitor visitor;
   visitor(program); 
}

void tac::Printer::printFunction(std::shared_ptr<tac::Function> function) const {
   DebugVisitor visitor;
   visitor(function); 
}

void tac::Printer::printStatement(tac::Statement& statement) const {
   DebugVisitor visitor;
   visit(visitor, statement);
}

void tac::Printer::printArgument(tac::Argument& arg) const {
    std::cout << printArg(arg) << std::endl;
}
