//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTERMEDIATE_PROGRAM_H
#define INTERMEDIATE_PROGRAM_H

#include <vector>
#include <memory>

#include "il/InstructionFactory.hpp"

namespace eddic {

class AssemblyFileWriter;
class Instruction;

class IntermediateProgram {
    private:
        std::vector<std::shared_ptr<Instruction>> m_instructions;
        InstructionFactory m_factory;

    public:
        InstructionFactory factory();
        void addInstruction(std::shared_ptr<Instruction> instruction);
        void writeAsm(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
