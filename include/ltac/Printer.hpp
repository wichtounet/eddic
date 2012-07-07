//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_PRINTER_H
#define LTAC_PRINTER_H

#include <memory>

#include "ltac/Program.hpp"

namespace eddic {

namespace ltac {

/*!
 * \class Printer
 * \brief Utility class to print the three-address-code representation on the console. 
 */
struct Printer {
    void print(std::shared_ptr<ltac::Program> program) const ;
    void print(std::shared_ptr<ltac::Function> function) const ;
    void print(ltac::Statement& statement) const ;
};

} //end of ltac

} //end of eddic

#endif
