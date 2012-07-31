//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRING_CONVERTER_H
#define STRING_CONVERTER_H

#include <string>

#include "assert.hpp"

#include "ltac/Argument.hpp"
#include "ltac/Address.hpp"
#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"

namespace eddic {

namespace as {

struct StringConverter {
    std::string address_to_string(eddic::ltac::Address& address) const;

    virtual std::string operator()(ltac::Register& reg) const = 0;
    virtual std::string operator()(ltac::FloatRegister& reg) const = 0;
};

} //end of namespace as

} //end of namespace eddic

#endif
