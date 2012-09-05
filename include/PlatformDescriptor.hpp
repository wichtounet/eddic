//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PLATFORM_DESCRIPTOR_H
#define PLATFORM_DESCRIPTOR_H

#include <vector>

#include "BaseType.hpp"

namespace eddic {

struct PlatformDescriptor {
    virtual unsigned int size_of(BaseType type) const = 0;

    virtual unsigned int number_of_registers() const = 0;
    virtual unsigned int number_of_float_registers() const = 0;
    
    virtual std::vector<unsigned short> symbolic_registers() const = 0;
    virtual std::vector<unsigned short> symbolic_float_registers() const = 0;
    
    virtual unsigned short int_return_register1() const = 0;
    virtual unsigned short int_return_register2() const = 0;
    virtual unsigned short float_return_register() const = 0;
    
    virtual unsigned short a_register() const = 0;
    virtual unsigned short d_register() const = 0;

    virtual unsigned int number_of_variable_registers() const = 0;  
    virtual unsigned int number_of_float_variable_registers() const = 0;  
    
    virtual unsigned short int_variable_register(unsigned int position) const = 0;
    virtual unsigned short float_variable_register(unsigned int position) const = 0;
    
    virtual unsigned int numberOfIntParamRegisters() const = 0;  
    virtual unsigned int numberOfFloatParamRegisters() const = 0;  
    
    virtual unsigned short int_param_register(unsigned int position) const = 0;
    virtual unsigned short float_param_register(unsigned int position) const = 0;
};

} //end of eddic

#endif
