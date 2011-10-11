//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INSTRUCTION_FACTORY_H
#define INSTRUCTION_FACTORY_H

#include <string>
#include <memory>

#include "il/Instruction.hpp"

namespace eddic {

class InstructionFactory {
    public:
        std::shared_ptr<Instruction> createMove();
        std::shared_ptr<Instruction> createLabel(const std::string& label);
        std::shared_ptr<Instruction> createJump(const std::string& label);
        std::shared_ptr<Instruction> createCall(const std::string& function);
        std::shared_ptr<Instruction> createFunctionDeclaration(const std::string& function, int size = 0);
        std::shared_ptr<Instruction> createFunctionExit(int size = 0);
};

} //end of eddic

#endif
