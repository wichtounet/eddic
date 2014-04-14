//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_INSTRUCTION_H
#define LTAC_INSTRUCTION_H

#include <memory>
#include <vector>
#include <string>

#include <boost/optional.hpp>

#include "ltac/Operator.hpp"
#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"
#include "ltac/PseudoRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"
#include "ltac/Address.hpp"
#include "ltac/Argument.hpp"

#include "tac/Size.hpp"

namespace eddic {

class Function;

namespace ltac {

struct Instruction {
    private:
        std::size_t _uid;

    public:
        ltac::Operator op;
        boost::optional<Argument> arg1;
        boost::optional<Argument> arg2;
        boost::optional<Argument> arg3;
        tac::Size size;
    
        std::string label;                      //Only if jump
        eddic::Function* target_function;       //Only if a call

        std::vector<ltac::PseudoRegister> uses;
        std::vector<ltac::PseudoFloatRegister> float_uses;

        std::vector<ltac::Register> hard_uses;
        std::vector<ltac::FloatRegister> hard_float_uses;
    
        std::vector<ltac::PseudoRegister> kills;
        std::vector<ltac::PseudoFloatRegister> float_kills;

        std::vector<ltac::Register> hard_kills;
        std::vector<ltac::FloatRegister> hard_float_kills;

        //Instructions no param
        Instruction(Operator op, tac::Size = tac::Size::DEFAULT);

        //Instructions with unary operator
        Instruction(Operator op, Argument arg1, tac::Size = tac::Size::DEFAULT);

        //Instructions with binary operator
        Instruction(Operator op, Argument arg1, Argument arg2, tac::Size = tac::Size::DEFAULT);

        //Instructions with ternary operator
        Instruction(Operator op, Argument arg1, Argument arg2, Argument arg3, tac::Size = tac::Size::DEFAULT);
        
        //Jump, calls and labels
        Instruction(std::string label, Operator op, tac::Size = tac::Size::DEFAULT);

        //Copy constructors
        Instruction(const Instruction& rhs);
        Instruction& operator=(const Instruction& rhs);
        
        //Move constructrs
        Instruction(Instruction&& rhs) noexcept;
        Instruction& operator=(Instruction&& rhs) noexcept;

        std::size_t uid() const {
            return _uid;
        }

        bool is_jump() const;
        bool is_label() const;
};

} //end of ltac

} //end of eddic

#endif
