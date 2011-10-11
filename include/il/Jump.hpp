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

class Jump : public Instruction {
    private:
        std::string m_label;

    public:
        Jump(std::string label);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
