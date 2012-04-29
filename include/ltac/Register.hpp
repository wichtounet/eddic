//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_REGISTER_H
#define LTAC_REGISTER_H

namespace eddic {

namespace ltac {

struct Register {
    unsigned short reg;

    Register();
    Register(unsigned short);
    
    operator int();
};

static const Register ReturnInt1(100);
static const Register ReturnInt2(101);

static const Register SP(1000);
static const Register BP(1001);

static const unsigned short FirstIntParam = 500;

} //end of ltac

} //end of eddic

#endif
