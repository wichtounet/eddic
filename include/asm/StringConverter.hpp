//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
