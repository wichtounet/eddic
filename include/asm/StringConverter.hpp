//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef STRING_CONVERTER_H
#define STRING_CONVERTER_H

#include <string>

#include "ltac/forward.hpp"

namespace eddic {

namespace as {

struct StringConverter {
    std::string address_to_string(eddic::ltac::Address& address) const;
    std::string register_to_string(eddic::ltac::AddressRegister& reg) const;

    virtual std::string operator()(ltac::Register& reg) const = 0;
    virtual std::string operator()(ltac::FloatRegister& reg) const = 0;
    virtual std::string operator()(ltac::PseudoRegister& reg) const = 0;
    virtual std::string operator()(ltac::PseudoFloatRegister& reg) const = 0;
};

} //end of namespace as

} //end of namespace eddic

#endif
