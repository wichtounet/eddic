//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PLATFORM_DESCRIPTOR_H
#define PLATFORM_DESCRIPTOR_H

#include <vector>

namespace eddic {

struct PlatformDescriptor {
    virtual unsigned int numberOfIntParamRegisters() = 0;  
    virtual unsigned int numberOfFloatParamRegisters() = 0;  

    virtual unsigned int number_of_registers() = 0;
    virtual std::vector<unsigned short> symbolic_registers() = 0;

    virtual unsigned int number_of_float_registers() = 0;
    virtual std::vector<unsigned short> symbolic_float_registers() = 0;
};

} //end of eddic

#endif
