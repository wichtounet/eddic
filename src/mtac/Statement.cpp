//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/Printer.hpp"
#include "mtac/Statement.hpp"

using namespace eddic;

std::ostream& mtac::operator<<(std::ostream& out, std::shared_ptr<mtac::Statement> statement){
    if(statement){
        return out << *statement; 
    } else {
        return out << "null";
    }
}

std::ostream& mtac::operator<<(std::ostream& out, const mtac::Statement& statement){
    mtac::Printer printer;
    printer.print_inline(statement, out);

    return out;
}
