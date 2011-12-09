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
    }

    void operator()(tac::Quadruple& quadruple){

    }

    void operator()(tac::IfFalse& ifFalse){

    }

    void operator()(tac::Goto& goto_){
        std::cout << "\tgoto " << goto_.label << std::endl;
    }

    void operator()(tac::Param& param){

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
