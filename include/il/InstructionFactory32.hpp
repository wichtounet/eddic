//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef INSTRUCTION_FACTORY_32_H
#define INSTRUCTION_FACTORY_32_H

#include "il/InstructionFactory.hpp"

namespace eddic {

class Operand;

class InstructionFactory32 : public InstructionFactory {
    private:
        std::shared_ptr<Operand> registerOperands[REGISTER_COUNT];

    public:
        InstructionFactory32();

        std::shared_ptr<Instruction> createMove(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const;
        std::shared_ptr<Instruction> createCompare(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const;
        std::shared_ptr<Instruction> createPush(std::shared_ptr<Operand> operand) const;
        std::shared_ptr<Instruction> createPop(std::shared_ptr<Operand> operand) const;
        std::shared_ptr<Instruction> createLabel(const std::string& label) const;
        std::shared_ptr<Instruction> createJump(JumpCondition condition, const std::string& label) const;
        std::shared_ptr<Instruction> createCall(const std::string& function) const;
        std::shared_ptr<Instruction> createFunctionDeclaration(const std::string& function, int size = 0) const;
        std::shared_ptr<Instruction> createFunctionExit(int size = 0) const;
        std::shared_ptr<Instruction> createMath(Operation operation, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const;
        
        std::shared_ptr<Instruction> createGlobalIntVariable(const std::string& name, int value) const;
        std::shared_ptr<Instruction> createGlobalStringVariable(const std::string& name, const std::string& label, int size) const;
        std::shared_ptr<Instruction> createGlobalArray(const std::string& name, BaseType type, int size) const;
    
        std::shared_ptr<Instruction> createPrintStringFunction() const;
        std::shared_ptr<Instruction> createPrintIntegerFunction() const;
        std::shared_ptr<Instruction> createPrintLineFunction() const;
        std::shared_ptr<Instruction> createConcatFunction() const;
        std::shared_ptr<Instruction> createAllocFunction() const;

        std::shared_ptr<Instruction> createMainDeclaration() const;
        std::shared_ptr<Instruction> createDataSection(std::unordered_map<std::string, std::string> pool) const;
        
        std::shared_ptr<Operand> registers(Register reg) const;
};

} //end of eddic

#endif
