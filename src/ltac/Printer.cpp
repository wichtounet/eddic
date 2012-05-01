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

#include "VisitorUtils.hpp"
#include "Utils.hpp"

using namespace eddic;

namespace {

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
        return "address";
   }
   
   std::string operator()(std::string& str) const {
        return str;
   }
};

std::string printArg(ltac::Argument& arg){
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
        //TODO add arg3

        if(quadruple->arg1 && quadruple->arg2){
            std::cout << "\t" << static_cast<unsigned int>(quadruple->op) << " " << printArg(*quadruple->arg1) << ", " << printArg(*quadruple->arg2) << std::endl;
        } else if(quadruple->arg1){
            std::cout << "\t" << static_cast<unsigned int>(quadruple->op) << " " << printArg(*quadruple->arg1) << std::endl;
        } else {
            std::cout << "\t" << static_cast<unsigned int>(quadruple->op) << std::endl;
        }
    }

    void operator()(std::shared_ptr<ltac::Jump>& jmp){
        std::cout << "\tjmp (" << static_cast<unsigned int>(jmp->type) << ")" << jmp->label << std::endl;
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
