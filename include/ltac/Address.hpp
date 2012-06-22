//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_ADDRESS_H
#define LTAC_ADDRESS_H

#include <boost/optional.hpp>

#include "ltac/Register.hpp"

namespace eddic {

namespace ltac {

struct Address {
    boost::optional<ltac::Register> base_register;
    boost::optional<ltac::Register> scaled_register;
    boost::optional<unsigned int> scale;
    boost::optional<int> displacement;

    boost::optional<std::string> absolute;

    Address();
    Address(const std::string& absolute);
    Address(const std::string& absolute, ltac::Register reg);
    Address(const std::string& absolute, int displacement);
    
    Address(int displacement);
    Address(ltac::Register reg, int displacement);
    Address(ltac::Register reg, ltac::Register scaled);
    Address(ltac::Register reg, ltac::Register scaled, unsigned scale, int displacement);
};

bool operator==(ltac::Address& lhs, ltac::Address& rhs);
bool operator!=(ltac::Address& lhs, ltac::Address& rhs);

} //end of ltac

} //end of eddic

#endif
