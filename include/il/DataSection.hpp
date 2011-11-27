//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DATA_SECTION_H
#define DATA_SECTION_H

#include <string>
#include <unordered_map>

#include "il/Instruction.hpp"

namespace eddic {

class DataSection : public Instruction {
    private:
        std::unordered_map<std::string, std::string> m_pool;

    public:
        DataSection(std::unordered_map<std::string, std::string> pool);

        void write(AssemblyFileWriter& writer) const ;
};

} //end of eddic

#endif
