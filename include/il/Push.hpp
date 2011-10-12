//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PUSH_H
#define PUSH_H

#include <memory>

#include "Instruction.hpp"

namespace eddic {

class Operand;

class Push : public Instruction {
    private:
        std::shared_ptr<Operand> m_operand;

    public:
        Push(std::shared_ptr<Operand> operand);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
