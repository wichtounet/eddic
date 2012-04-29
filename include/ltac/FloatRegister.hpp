//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_FLOAT_REGISTER_H
#define LTAC_FLOAT_REGISTER_H

namespace eddic {

namespace ltac {

struct FloatRegister {
    unsigned short reg;

    FloatRegister(unsigned short);

    operator int();
};

} //end of ltac

} //end of eddic

#endif
