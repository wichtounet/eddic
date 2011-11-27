//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GLOBAL_INT_VARIABLE_H
#define GLOBAL_INT_VARIABLE_H

#include <string>

#include "Instruction.hpp"

namespace eddic {

class GlobalIntVariable : public Instruction {
    private:
        std::string m_name;
        int m_value;

    public:
        GlobalIntVariable(const std::string& name, int value);

        void write(AssemblyFileWriter& writer) const ;
};

} //end of eddic

#endif
