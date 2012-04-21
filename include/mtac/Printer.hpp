//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_PRINTER_H
#define MTAC_PRINTER_H

#include <memory>

#include "mtac/Statement.hpp"
#include "mtac/Argument.hpp"
#include "mtac/Function.hpp"

namespace eddic {

namespace mtac {

class Program;

/*!
 * \class Printer
 * \brief Utility class to print the three-address-code representation on the console. 
 */
struct Printer {
    void print(mtac::Program& program) const ;
    void printFunction(std::shared_ptr<mtac::Function> function) const ;
    void printStatement(mtac::Statement& program) const ;
    void printArgument(mtac::Argument& program) const ;
};

} //end of mtac

} //end of eddic

#endif
