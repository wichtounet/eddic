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
#include <unordered_map>

#include "Types.hpp"

#include "il/Instruction.hpp"
#include "il/Math.hpp"
#include "il/Jump.hpp"
#include "il/Register.hpp"

namespace eddic {

class Operand;

class InstructionFactory {
    public:
        virtual std::shared_ptr<Instruction> createMove(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const = 0;
        virtual std::shared_ptr<Instruction> createCompare(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const = 0;
        virtual std::shared_ptr<Instruction> createPush(std::shared_ptr<Operand> operand) const = 0;
        virtual std::shared_ptr<Instruction> createPop(std::shared_ptr<Operand> operand) const = 0;
        virtual std::shared_ptr<Instruction> createLabel(const std::string& label) const = 0;
        virtual std::shared_ptr<Instruction> createJump(JumpCondition condition, const std::string& label) const = 0;
        virtual std::shared_ptr<Instruction> createCall(const std::string& function) const = 0;
        virtual std::shared_ptr<Instruction> createFunctionDeclaration(const std::string& function, int size = 0) const = 0;
        virtual std::shared_ptr<Instruction> createFunctionExit(int size = 0) const = 0;
        virtual std::shared_ptr<Instruction> createMath(Operation operation, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const = 0;
        
        virtual std::shared_ptr<Instruction> createGlobalIntVariable(std::string name, int value) const = 0;
        virtual std::shared_ptr<Instruction> createGlobalStringVariable(std::string name, std::string label, int size) const = 0;
        virtual std::shared_ptr<Instruction> createGlobalArray(std::string name, BaseType type, int size) const = 0;
        
        virtual std::shared_ptr<Instruction> createPrintStringFunction() const = 0;
        virtual std::shared_ptr<Instruction> createPrintIntegerFunction() const = 0;
        virtual std::shared_ptr<Instruction> createPrintLineFunction() const = 0;
        virtual std::shared_ptr<Instruction> createConcatFunction() const = 0;
        virtual std::shared_ptr<Instruction> createAllocFunction() const = 0;

        virtual std::shared_ptr<Instruction> createMainDeclaration() const = 0;
        virtual std::shared_ptr<Instruction> createDataSection(std::unordered_map<std::string, std::string> pool) const = 0;

        virtual std::shared_ptr<Operand> registers(Register reg) const = 0;
};

} //end of eddic

#endif
