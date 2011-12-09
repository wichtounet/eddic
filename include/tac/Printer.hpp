//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_PRINTER_H
#define TAC_PRINTER_H

namespace eddic {

namespace tac {

class Program;

/*!
 * \class Printer
 * \brief Utility class to print the three-address-code representation on the console. 
 */
struct Printer {
    void print(tac::Program& program) const ;
};

} //end of tac

} //end of eddic

#endif
