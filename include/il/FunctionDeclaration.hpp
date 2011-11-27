//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_DECLARATION_H
#define FUNCTION_DECLARATION_H

#include <string>

#include "il/Instruction.hpp"

namespace eddic {

class AssemblyFileWriter;

class FunctionDeclaration : public Instruction {
    private:
        std::string m_name;
        int m_size;

    public:
        FunctionDeclaration(const std::string& name, int size = 0);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
