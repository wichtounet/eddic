//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_ADDRESS_H
#define LTAC_ADDRESS_H

#include <boost/optional.hpp>

namespace eddic {

namespace ltac {

struct Register;

struct Address {
    boost::optional<unsigned int> displacement;
    boost::optional<unsigned int> scale;
    boost::optional<Register> base_register;
    boost::optional<Register> scaled_register;
};

} //end of ltac

} //end of eddic

#endif
