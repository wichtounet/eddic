//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef COMPARE_H
#define COMPARE_H

#include <memory>

#include "il/Instruction.hpp"

namespace eddic {

class Operand;

class Compare : public Instruction {
    private:
        std::shared_ptr<Operand> m_lhs;
        std::shared_ptr<Operand> m_rhs;

    public:
        Compare(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs);

        void write(AssemblyFileWriter& writer) const ;
};

} //end of eddic

#endif
