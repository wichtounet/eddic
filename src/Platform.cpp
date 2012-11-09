//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Platform.hpp"

using namespace eddic;

struct X86Descriptor : public PlatformDescriptor {
    int sizes[BASETYPE_COUNT] = {  8, 4, 4, 4, 4, 0 };

    X86Descriptor(){};

    unsigned int size_of(BaseType type) const {
        return sizes[static_cast<unsigned int>(type)];
    }

    unsigned int numberOfIntParamRegisters() const {
        return 1;
    }
      
    unsigned int numberOfFloatParamRegisters() const {
        return 1;
    }

    unsigned int number_of_registers() const {
        return 6;
    }
    
    std::vector<unsigned short> symbolic_registers() const {
        return {0, 1, 2, 3, 4, 5};
    }
    
    unsigned int number_of_float_registers() const {
        return 8;
    }
    
    std::vector<unsigned short> symbolic_float_registers() const {
        return {0, 1, 2, 3, 4, 5, 6, 7};
    }

    unsigned int number_of_variable_registers() const {
        return 0;
    }

    unsigned int number_of_float_variable_registers() const {
        return 1;
    }
    
    unsigned short int_variable_register(unsigned int /*position*/) const {
        eddic_unreachable("No int variable register");

        return 0;
    }

    unsigned short float_variable_register(unsigned int position) const {
        assert(position == 1);

        return 6; //xmm6
    };

    unsigned short int_param_register(unsigned int position) const {
        assert(position == 1);

        return 2; //ecx
    }

    unsigned short float_param_register(unsigned int position) const {
        assert(position  == 1);

        return 7; //xmm7
    }
    
    unsigned short int_return_register1() const {
        return 0;
    }

    unsigned short int_return_register2() const {
        return 1;
    }
    
    unsigned short float_return_register() const {
        return 0;
    }
    
    unsigned short a_register() const {
        return 0;
    }

    unsigned short d_register() const {
        return 3;
    }
};

struct X86_64Descriptor : public PlatformDescriptor {
    int sizes[BASETYPE_COUNT] = { 16, 8, 8, 8, 8, 0 };

    X86_64Descriptor(){};

    unsigned int size_of(BaseType type) const {
        return sizes[static_cast<unsigned int>(type)];
    }

    unsigned int numberOfIntParamRegisters() const {
        return 2;
    }
      
    unsigned int numberOfFloatParamRegisters() const {
        return 1;
    }
    
    unsigned int number_of_registers() const {
        return 14;
    }
    
    std::vector<unsigned short> symbolic_registers() const {
        return {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 3, 2, 1, 0};
    }
    
    unsigned int number_of_float_registers() const {
        return 8;
    }
    
    std::vector<unsigned short> symbolic_float_registers() const {
        return {0, 1, 2, 3, 4, 5, 6, 7};
    }
    
    unsigned int number_of_variable_registers() const {
        return 2;
    }

    unsigned int number_of_float_variable_registers() const {
        return 1;
    }
    
    unsigned short int_variable_register(unsigned int position) const {
        assert(position == 1 || position == 2);

        return position == 1 ? 10 : 11; //r12 and r13
    }

    unsigned short float_variable_register(unsigned int position) const {
        assert(position == 1);

        return 6; //xmm6
    };

    unsigned short int_param_register(unsigned int position) const {
        assert(position == 1 || position == 2);

        return position == 1 ? 12 : 13; //r14 and r15
    }

    unsigned short float_param_register(unsigned int position) const {
        assert(position  == 1);

        return 7;
    }
    
    unsigned short int_return_register1() const {
        return 0;
    }

    unsigned short int_return_register2() const {
        return 1;
    }
    
    unsigned short float_return_register() const {
        return 0;
    }
    
    unsigned short a_register() const {
        return 0;
    }

    unsigned short d_register() const {
        return 3;
    }
};
    
static const X86Descriptor x86Descriptor;
static const X86_64Descriptor x86_64Descriptor;

const PlatformDescriptor* eddic::getPlatformDescriptor(Platform platform){
    switch(platform){
        case Platform::INTEL_X86:
            return &x86Descriptor;
        case Platform::INTEL_X86_64:
            return &x86_64Descriptor;
    }

    eddic_unreachable("Unhandled platform");
}
