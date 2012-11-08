//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "ltac/Printer.hpp"
#include "ltac/Statement.hpp"

using namespace eddic;

std::ostream& ltac::operator<<(std::ostream& out, std::shared_ptr<ltac::Statement> statement){
    if(statement){
        return out << *statement; 
    } else {
        return out << "null";
    }
}

std::ostream& ltac::operator<<(std::ostream& out, const ltac::Statement& statement){
    ltac::print_statement(statement, out);

    return out;
}
