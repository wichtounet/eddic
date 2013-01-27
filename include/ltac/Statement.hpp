//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_STATEMENT_H
#define LTAC_STATEMENT_H

#include "ltac/Instruction.hpp"

namespace eddic {

namespace ltac {

typedef 
        std::shared_ptr<ltac::Instruction>        //Basic quadruples
     Statement;

std::ostream& operator<<(std::ostream& out, const Statement& statement);
std::ostream& operator<<(std::ostream& out, std::shared_ptr<Statement> statement);

} //end of ltac

} //end of eddic

#endif
