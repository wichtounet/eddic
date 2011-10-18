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
#include "il/Math.hpp"
#include "il/Jump.hpp"

namespace eddic {

class Operand;

class InstructionFactory {
    public:
        std::shared_ptr<Instruction> createMove(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs);
        std::shared_ptr<Instruction> createCompare(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs);
        std::shared_ptr<Instruction> createPush(std::shared_ptr<Operand> operand);
        std::shared_ptr<Instruction> createLabel(const std::string& label);
        std::shared_ptr<Instruction> createJump(JumpCondition condition, const std::string& label);
        std::shared_ptr<Instruction> createCall(const std::string& function);
        std::shared_ptr<Instruction> createFunctionDeclaration(const std::string& function, int size = 0);
        std::shared_ptr<Instruction> createFunctionExit(int size = 0);
        std::shared_ptr<Instruction> createMath(Operation operation, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs);
};

} //end of eddic

#endif
