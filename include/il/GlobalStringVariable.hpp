//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GLOBAL_STRING_VARIABLE_H
#define GLOBAL_STRING_VARIABLE_H

#include <string>

#include "Instruction.hpp"

namespace eddic {

class GlobalStringVariable : public Instruction {
    private:
        std::string m_name;
        std::string m_label;
        int m_size;

    public:
        GlobalStringVariable(std::string name, std::string label, int size);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
