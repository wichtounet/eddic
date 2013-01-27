//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_FORWARD_DECLARATIONS_H
#define LTAC_FORWARD_DECLARATIONS_H

#include <memory>

#include "variant.hpp"

namespace eddic {

namespace ltac {

struct Instruction;

struct Register;
struct FloatRegister;
struct PseudoRegister;
struct PseudoFloatRegister;

typedef 
        std::shared_ptr<ltac::Instruction>         //Basic quadruples
     Statement;

typedef boost::variant<
            ltac::Register, 
            ltac::PseudoRegister, 
            ltac::PseudoFloatRegister,  //Not used
            ltac::FloatRegister>        //Not used
    AddressRegister;

struct Address;

} //end of ltac

} //end of eddic

#endif
