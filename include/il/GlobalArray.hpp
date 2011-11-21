//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GLOBAL_ARRAY_H
#define GLOBAL_ARRAY_H

#include <string>

#include "Instruction.hpp"
#include "Types.hpp"

namespace eddic {

class GlobalArray : public Instruction {
    private:
        std::string name;
        BaseType type;
        int size;

    public:
        GlobalArray(std::string name, BaseType type, int size);

        void write(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
