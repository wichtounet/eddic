//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LABEL_H
#define LABEL_H

#include <string>

#include "Instruction.hpp"

namespace eddic {

class Label : public Instruction {
    private:
        std::string m_name;

    public:
        Label(const std::string& name);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
