//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_PRINTER_H
#define MTAC_PRINTER_H

#include "mtac/forward.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

namespace mtac {

/*!
 * \class Printer
 * \brief Utility class to print the three-address-code representation on the console. 
 */
struct Printer {
    void print(mtac::Program& program) const ;
};

void print(mtac::Program& program);

} //end of mtac

} //end of eddic

#endif
