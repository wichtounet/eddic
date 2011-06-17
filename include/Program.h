//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>

#include "Instruction.h"

class Program {
  private:
    std::vector<Instruction*> instructions;
  public:
    void addInstruction(Instruction* instruction);
    std::vector<Instruction*>::iterator begin();
    std::vector<Instruction*>::iterator end();
};

#endif
