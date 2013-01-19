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
            stream << function;
        }
    }
};

} //end of anonymous namespace

void mtac::Printer::print(mtac::Program& program) const {
   DebugVisitor visitor;
   visitor.print(program);
}

void mtac::print(mtac::Program& program){
    mtac::Printer printer;
    printer.print(program);
}
