//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_PRINTER_H
#define LTAC_PRINTER_H

#include <iostream>

#include "mtac/forward.hpp"
#include "ltac/forward.hpp"

namespace eddic {

namespace ltac {

/*!
 * \class Printer
 * \brief Utility class to print the three-address-code representation on the console. 
 */
struct Printer {
    void print(mtac::Program& program) const ;
    void print(mtac::Function& function) const ;
    void print(ltac::Instruction& statement) const ;
};

void print_statement(const ltac::Instruction& statement, std::ostream& out = std::cout);

} //end of ltac

} //end of eddic

#endif
