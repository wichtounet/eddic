//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef POP_H
#define POP_H

#include <memory>

#include "il/Instruction.hpp"

namespace eddic {

class Operand;

class Pop : public Instruction {
    private:
        std::shared_ptr<Operand> operand;

    public:
        Pop(std::shared_ptr<Operand> operand);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
