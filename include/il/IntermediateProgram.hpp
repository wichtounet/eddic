//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INTERMEDIATE_PROGRAM_H
#define INTERMEDIATE_PROGRAM_H

#include <vector>

namespace eddic {

class AssemblyFileWriter;
class Instruction;

class IntermediateProgram {
    private:
        std::vector<Instruction> m_instructions;
        InstructionFactory m_factory;

    public:
        InstructionFactory factory();
        void addInstruction(Instruction instruction);
        void writeAsm(AssemblyFileWriter& writer);
};

} //end of eddic

#endif
