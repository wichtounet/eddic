//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_STATEMENT_H
#define LTAC_STATEMENT_H

#include <boost/variant/variant.hpp>

#include "ltac/Instruction.hpp"
#include "ltac/Jump.hpp"
#include "ltac/Call.hpp"

namespace eddic {

namespace ltac {

typedef boost::variant<
        std::shared_ptr<ltac::Instruction>,        //Basic quadruples
        std::shared_ptr<ltac::Jump>,             //Jumps
        std::shared_ptr<ltac::Call>,             //Call a function
        std::string                             //For labels
    > Statement;

} //end of ltac

} //end of eddic

#endif
