//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_PRINTER_H
#define LTAC_PRINTER_H

#include <memory>
#include <iostream>

#include "mtac/Program.hpp"

namespace eddic {

namespace ltac {

/*!
 * \class Printer
 * \brief Utility class to print the three-address-code representation on the console. 
 */
struct Printer {
    void print(std::shared_ptr<mtac::Program> program) const ;
    void print(mtac::function_p function) const ;
    void print(ltac::Statement& statement) const ;
};

void print_statement(const ltac::Statement& statement, std::ostream& out = std::cout);

} //end of ltac

} //end of eddic

#endif
