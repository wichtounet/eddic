//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_ABSTRACT_REGISTER_MANAGER_H
#define LTAC_ABSTRACT_REGISTER_MANAGER_H

#include <memory>
#include <vector>

#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"

#include "asm/Registers.hpp"

namespace eddic {

class Variable;

namespace ltac {

class AbstractRegisterManager {
    protected:
        AbstractRegisterManager(const std::vector<ltac::Register>& registers, const std::vector<ltac::FloatRegister>& float_registers);

    public:
        bool is_reserved(ltac::Register reg);
        bool is_reserved(ltac::FloatRegister reg);
        void reserve(ltac::Register reg);
        void reserve(ltac::FloatRegister reg);
        void release(ltac::Register reg);
        void release(ltac::FloatRegister reg);
        
        bool in_register(std::shared_ptr<Variable> variable, ltac::Register reg);
        bool in_register(std::shared_ptr<Variable> variable, ltac::FloatRegister reg);
        
        void setLocation(std::shared_ptr<Variable> variable, ltac::Register reg);
        void setLocation(std::shared_ptr<Variable> variable, ltac::FloatRegister reg);
    
    protected:
        //The registers
        as::Registers<ltac::Register> registers;
        as::Registers<ltac::FloatRegister> float_registers;
};

} //end of ltac

} //end of eddic

#endif
