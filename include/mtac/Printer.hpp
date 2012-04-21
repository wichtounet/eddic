//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_PRINTER_H
#define TAC_PRINTER_H

#include <memory>

#include "tac/Statement.hpp"
#include "tac/Argument.hpp"
#include "tac/Function.hpp"

namespace eddic {

namespace tac {

class Program;

/*!
 * \class Printer
 * \brief Utility class to print the three-address-code representation on the console. 
 */
struct Printer {
    void print(tac::Program& program) const ;
    void printFunction(std::shared_ptr<tac::Function> function) const ;
    void printStatement(tac::Statement& program) const ;
    void printArgument(tac::Argument& program) const ;
};

} //end of tac

} //end of eddic

#endif
