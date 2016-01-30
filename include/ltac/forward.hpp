//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
