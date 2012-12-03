//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_REGISTER_MANAGER_H
#define LTAC_REGISTER_MANAGER_H

#include <memory>
#include <vector>
#include <unordered_set>

#include "Platform.hpp"
#include "FloatPool.hpp"

#include "mtac/EscapeAnalysis.hpp"
#include "mtac/forward.hpp"
#include "mtac/Argument.hpp"

//Forward is not enough for PseudoRegisters
#include "ltac/Argument.hpp"
#include "ltac/Operator.hpp"
#include "ltac/PseudoRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"

#include "asm/PseudoRegisters.hpp"

namespace eddic {

namespace ltac {

class StatementCompiler;

class RegisterManager {
    public:
        std::unordered_set<std::shared_ptr<Variable>> written;
        std::unordered_set<std::shared_ptr<Variable>> local;

        mtac::EscapedVariables pointer_escaped;

        mtac::basic_block_p bb;

        RegisterManager(mtac::function_p function, std::shared_ptr<FloatPool> float_pool);

        /*!
         * Deleted copy constructor
         */
        RegisterManager(const RegisterManager& rhs) = delete;

        /*!
         * Deleted copy assignment operator. 
         */
        RegisterManager& operator=(const RegisterManager& rhs) = delete;

        void reset();

        ltac::PseudoRegister get_pseudo_reg(std::shared_ptr<Variable> var);
        ltac::PseudoRegister get_pseudo_reg_no_move(std::shared_ptr<Variable> var);
        ltac::PseudoFloatRegister get_pseudo_float_reg(std::shared_ptr<Variable> var);
        ltac::PseudoFloatRegister get_pseudo_float_reg_no_move(std::shared_ptr<Variable> var);

        void copy(mtac::Argument argument, ltac::PseudoFloatRegister reg);
        void copy(mtac::Argument argument, ltac::PseudoRegister reg);

        void move(mtac::Argument argument, ltac::PseudoRegister reg);
        void move(mtac::Argument argument, ltac::PseudoFloatRegister reg);

        ltac::PseudoRegister get_free_pseudo_reg();
        ltac::PseudoFloatRegister get_free_pseudo_float_reg();
        
        ltac::PseudoRegister get_bound_pseudo_reg(unsigned short hard);
        ltac::PseudoFloatRegister get_bound_pseudo_float_reg(unsigned short hard);

        bool is_written(std::shared_ptr<Variable> variable);
        void set_written(std::shared_ptr<Variable> variable);

        bool is_escaped(std::shared_ptr<Variable> variable);

        void collect_parameters(std::shared_ptr<eddic::Function> definition, const PlatformDescriptor* descriptor);

        int last_pseudo_reg();
        int last_float_pseudo_reg();

        void remove_from_pseudo_reg(std::shared_ptr<Variable> variable);
        void remove_from_pseudo_float_reg(std::shared_ptr<Variable> variable);
    
    private: 
        //Allow to push needed register before the first push param
        bool first_param = true;
        
        mtac::function_p function;

        std::shared_ptr<FloatPool> float_pool;

        //The pseudo registers
        as::PseudoRegisters<ltac::PseudoRegister> pseudo_registers;
        as::PseudoRegisters<ltac::PseudoFloatRegister> pseudo_float_registers;
};

} //end of ltac

} //end of eddic

#endif
