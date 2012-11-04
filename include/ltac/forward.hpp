//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_FORWARD_DECLARATIONS_H
#define LTAC_FORWARD_DECLARATIONS_H

#include <memory>
#include <string>

#include "variant.hpp"

namespace eddic {

namespace ltac {

struct Instruction;
struct Jump;

struct Register;
struct FloatRegister;
struct PseudoRegister;
struct PseudoFloatRegister;

typedef boost::variant<
        std::shared_ptr<ltac::Instruction>,         //Basic quadruples
        std::shared_ptr<ltac::Jump>,                //Jumps
        std::string                                 //For labels
    > Statement;

} //end of ltac

} //end of eddic

#endif
