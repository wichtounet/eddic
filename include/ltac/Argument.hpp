//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_ARGUMENT_H
#define LTAC_ARGUMENT_H

#include <string>

#include "variant.hpp"

#include "ltac/forward.hpp"

namespace eddic {

namespace ltac {

typedef boost::variant<
        /* Hard Registers */
        eddic::ltac::FloatRegister, eddic::ltac::Register, 
        /* Pseudo Registers */
        eddic::ltac::PseudoFloatRegister, eddic::ltac::PseudoRegister, 
        /* Address */
        eddic::ltac::Address, 
        /* Literals */
        std::string, 
        /* Constants */
        double, int
    > Argument;

} //end of ltac

} //end of eddic

#endif
