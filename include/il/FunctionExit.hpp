//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_EXIT_H
#define FUNCTION_EXIT_H

#include "Instruction.hpp"

namespace eddic {

class AssemblyFileWriter;

class FunctionExit : public Instruction {
    private:
        int m_size;

    public:
        FunctionExit(int size);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
