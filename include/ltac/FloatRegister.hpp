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

    FloatRegister();
    FloatRegister(unsigned short);

    operator int();

    bool operator==(const FloatRegister& rhs) const;
    bool operator!=(const FloatRegister& rhs) const;
};

static const FloatRegister ReturnFloat(100);

static const unsigned short FirstFloatParam = 500;

} //end of ltac

} //end of eddic

#endif
