//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef JUMP_H
#define JUMP_H

#include <string>

#include "Instruction.hpp"

namespace eddic {

enum class JumpCondition : unsigned int {
    ALWAYS,
    LESS,
    GREATER,
    EQUALS,
    NOT_EQUALS,
    GREATER_EQUALS,
    LESS_EQUALS
};

class Jump : public Instruction {
    private:
        JumpCondition m_condition;
        std::string m_label;

    public:
        Jump(JumpCondition condition, const std::string& label);

        void write(AssemblyFileWriter& writer) const ;
};

} //end of eddic

#endif
