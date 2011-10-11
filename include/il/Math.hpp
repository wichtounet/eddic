//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ADD_H
#define ADD_H

#include <memory>

#include "il/Instruction.hpp"
#include "il/Operand.hpp"

namespace eddic {

enum class Operation : unsigned int {
    ADD,
    SUB,
    DIV,
    MUL,
    MODULO
};

class Math : public Instruction {
    private:
        Operation m_operation;
        std::shared_ptr<Operand> m_lhs;
        std::shared_ptr<Operand> m_rhs;

    public:
        Math(Operation operation, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
