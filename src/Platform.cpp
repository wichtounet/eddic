//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "Platform.hpp"

//32 bits by default
eddic::Platform eddic::platform = Platform::INTEL_X86;

using namespace eddic;

struct X86Descriptor : public PlatformDescriptor {
    unsigned int numberOfIntParamRegisters(){
        return 1;
    }
      
    unsigned int numberOfFloatParamRegisters(){
        return 1;
    }

    unsigned int number_of_registers(){
        return 6;
    }
    
    std::vector<unsigned short> symbolic_registers(){
        return {1, 2, 3, 4, 5, 6};
    }
    
    unsigned int number_of_float_registers(){
        return 8;
    }
    
    std::vector<unsigned short> symbolic_float_registers(){
        return {0, 1, 2, 3, 4, 5, 6, 7};
    }
};

struct X86_64Descriptor : public PlatformDescriptor {
    unsigned int numberOfIntParamRegisters(){
        return 2;
    }
      
    unsigned int numberOfFloatParamRegisters(){
        return 1;
    }
    
    unsigned int number_of_registers(){
        return 6;
    }
    
    std::vector<unsigned short> symbolic_registers(){
        return {1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    }
    
    unsigned int number_of_float_registers(){
        return 8;
    }
    
    std::vector<unsigned short> symbolic_float_registers(){
        return {0, 1, 2, 3, 4, 5, 6, 7};
    }
};

PlatformDescriptor* eddic::getPlatformDescriptor(Platform platform){
    static X86Descriptor x86Descriptor;
    static X86_64Descriptor x86_64Descriptor;
    
    switch(platform){
        case Platform::INTEL_X86:
            return &x86Descriptor;
        case Platform::INTEL_X86_64:
            return &x86_64Descriptor;
    }

    assert(false);
}
