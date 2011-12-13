//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

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

   std::string operator()(std::string& str) const {
        return str;
   }
};

std::string printArgument(tac::Argument& arg){
    return boost::apply_visitor(ArgumentToString(), arg);
}

struct DebugVisitor : public boost::static_visitor<> {
    void operator()(tac::Program& program){
        std::cout << "TAC Program " << std::endl << std::endl; 

        visit_each_non_variant(*this, program.functions);
    }

    void operator()(std::shared_ptr<tac::Function> function){
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

    void operator()(tac::Quadruple& quadruple){
        if(!quadruple.op){
            std::cout << "\t" << quadruple.result->name() << " = " << printArgument(quadruple.arg1) << std::endl;
        } else {
            tac::Operator op = *quadruple.op;

            if(op == tac::Operator::ADD){
                std::cout << "\t" << quadruple.result->name() << " = " << printArgument(quadruple.arg1) << " + " << printArgument(*quadruple.arg2) << std::endl;
            } else if(op == tac::Operator::SUB){
                std::cout << "\t" << quadruple.result->name() << " = " << printArgument(quadruple.arg1) << " - " << printArgument(*quadruple.arg2) << std::endl;
            } else if(op == tac::Operator::MUL){
                std::cout << "\t" << quadruple.result->name() << " = " << printArgument(quadruple.arg1) << " * " << printArgument(*quadruple.arg2) << std::endl;
            } else if(op == tac::Operator::DIV){
                std::cout << "\t" << quadruple.result->name() << " = " << printArgument(quadruple.arg1) << " / " << printArgument(*quadruple.arg2) << std::endl;
            } else if(op == tac::Operator::MOD){
                std::cout << "\t" << quadruple.result->name() << " = " << printArgument(quadruple.arg1) << " % " << printArgument(*quadruple.arg2) << std::endl;
            } else if(op == tac::Operator::DOT){
                std::cout << "\t" << quadruple.result->name() << " = (" << printArgument(quadruple.arg1) << ")" << printArgument(*quadruple.arg2) << std::endl;
            } else if(op == tac::Operator::DOT_ASSIGN){
                std::cout << "\t(" << quadruple.result->name() << ")" << printArgument(quadruple.arg1) << " = " << printArgument(*quadruple.arg2) << std::endl;
            } else if(op == tac::Operator::ARRAY){
                std::cout << "\t" << quadruple.result->name() << " = " << printArgument(quadruple.arg1) << " [" << printArgument(*quadruple.arg2) << "]" << std::endl;
            } else if(op == tac::Operator::ARRAY_ASSIGN){
                std::cout << "\t" << quadruple.result->name() << "[" << printArgument(quadruple.arg1) << "] = " << printArgument(*quadruple.arg2) << std::endl;
            }
        }
    }

    template<typename T>
    std::string printTarget(T& ifFalse){
        if(ifFalse.block){
            return "B" + toString(ifFalse.block->index);   
        } else {
            return ifFalse.label;
        }
    }

    void operator()(tac::IfFalse& ifFalse){
        if(ifFalse.op == tac::BinaryOperator::EQUALS){
            std::cout << "\tifFalse " << printArgument(ifFalse.arg1) << " == " << printArgument(ifFalse.arg2) << " goto " << printTarget(ifFalse) << std::endl;
        } else if(ifFalse.op == tac::BinaryOperator::NOT_EQUALS){
            std::cout << "\tifFalse " << printArgument(ifFalse.arg1) << " != " << printArgument(ifFalse.arg2) << " goto " << printTarget(ifFalse) << std::endl;
        } else if(ifFalse.op == tac::BinaryOperator::LESS){
            std::cout << "\tifFalse " << printArgument(ifFalse.arg1) << " < " << printArgument(ifFalse.arg2) << " goto " << printTarget(ifFalse) << std::endl;
        } else if(ifFalse.op == tac::BinaryOperator::LESS_EQUALS){
            std::cout << "\tifFalse " << printArgument(ifFalse.arg1) << " <= " << printArgument(ifFalse.arg2) << " goto " << printTarget(ifFalse) << std::endl;
        } else if(ifFalse.op == tac::BinaryOperator::GREATER){
            std::cout << "\tifFalse " << printArgument(ifFalse.arg1) << " > " << printArgument(ifFalse.arg2) << " goto " << printTarget(ifFalse) << std::endl;
        } else if(ifFalse.op == tac::BinaryOperator::GREATER_EQUALS){
            std::cout << "\tifFalse " << printArgument(ifFalse.arg1) << " >= " << printArgument(ifFalse.arg2) << " goto " << printTarget(ifFalse) << std::endl;
        }
    }

    void operator()(tac::Goto& goto_){
        std::cout << "\tgoto " << printTarget(goto_) << std::endl;
    }

    void operator()(tac::Param& param){
        std::cout << "\tparam ";

        std::cout << printArgument(param.arg);

        std::cout << std::endl;
    }

    void operator()(tac::Return& return_){
        std::cout << "\treturn";

        if(return_.arg1){
            std::cout << " " << printArgument(*return_.arg1);
        }

        if(return_.arg2){
            std::cout << ", " << printArgument(*return_.arg2);
        }

        std::cout << std::endl;
    }

    void operator()(tac::Call& call){
        std::cout << "\t";

        if(call.return_){
            std::cout << call.return_->name();
        }

        if(call.return2_){
            std::cout << ", " << call.return2_->name();
        }

        if(call.return_ || call.return2_){
            std::cout << " = ";
        }

        std::cout << "call " << call.function << std::endl;
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
