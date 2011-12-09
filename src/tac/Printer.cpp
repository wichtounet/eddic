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

using namespace eddic;

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
        std::cout << "goto " << goto_.label << std::endl;
    }

    void operator()(tac::Param& param){

    }

    void operator()(tac::Return& return_){

    }

    void operator()(tac::Call& call){

    }

    void operator()(std::string& label){
        std::cout << label << ":" << std::endl;
    }
};

void tac::Printer::print(tac::Program& program) const {
   DebugVisitor visitor;
   visitor(program); 
}
