//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/InstructionFactory32.hpp"
#include "il/Instruction.hpp"
#include "il/Label.hpp"
#include "il/Call.hpp"
#include "il/Jump.hpp"
#include "il/FunctionDeclaration.hpp"
#include "il/FunctionExit.hpp"
#include "il/Operand.hpp"
#include "il/Move.hpp"
#include "il/Compare.hpp"
#include "il/Push.hpp"
#include "il/GlobalStringVariable.hpp"
#include "il/GlobalIntVariable.hpp"
#include "il/GlobalArray.hpp"
#include "il/PrintLineFunction.hpp"
#include "il/PrintStringFunction.hpp"
#include "il/PrintIntegerFunction.hpp"
#include "il/ConcatFunction.hpp"
#include "il/Main.hpp"
#include "il/DataSection.hpp"

#include "il/RegisterOperand.hpp"

using namespace eddic;

InstructionFactory32::InstructionFactory32(){
    registerOperands[EAX] = std::make_shared<RegisterOperand>("%eax");
    registerOperands[EBX] = std::make_shared<RegisterOperand>("%ebx");
    registerOperands[ECX] = std::make_shared<RegisterOperand>("%ecx");
    registerOperands[EDX] = std::make_shared<RegisterOperand>("%edx");
    registerOperands[ESP] = std::make_shared<RegisterOperand>("%esp");
    registerOperands[EBP] = std::make_shared<RegisterOperand>("%ebp");
}

std::shared_ptr<Instruction> InstructionFactory32::createMove(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const {
   return std::make_shared<Move>(lhs, rhs); 
}

std::shared_ptr<Instruction> InstructionFactory32::createCompare(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const {
   return std::make_shared<Compare>(lhs, rhs); 
}

std::shared_ptr<Instruction> InstructionFactory32::createPush(std::shared_ptr<Operand> operand) const {
   return std::make_shared<Push>(operand); 
}

std::shared_ptr<Instruction> InstructionFactory32::createLabel(const std::string& label) const {
   return std::make_shared<Label>(label); 
}

std::shared_ptr<Instruction> InstructionFactory32::createJump(JumpCondition condition, const std::string& label) const {
   return std::make_shared<Jump>(condition, label); 
}

std::shared_ptr<Instruction> InstructionFactory32::createCall(const std::string& function) const {
   return std::make_shared<Call>(function); 
}

std::shared_ptr<Instruction> InstructionFactory32::createFunctionDeclaration(const std::string& function, int size) const {
   return std::make_shared<FunctionDeclaration>(function, size); 
}

std::shared_ptr<Instruction> InstructionFactory32::createFunctionExit(int size) const {
   return std::make_shared<FunctionExit>(size); 
}

std::shared_ptr<Instruction> InstructionFactory32::createMath(Operation operation, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs) const {
   return std::make_shared<Math>(operation, lhs, rhs); 
}

std::shared_ptr<Instruction> InstructionFactory32::createGlobalIntVariable(std::string name, int value) const {
   return std::make_shared<GlobalIntVariable>(name, value); 
}

std::shared_ptr<Instruction> InstructionFactory32::createGlobalStringVariable(std::string name, std::string label, int size) const {
    return std::make_shared<GlobalStringVariable>(name, label, size); 
}

std::shared_ptr<Instruction> InstructionFactory32::createGlobalArray(std::string name, BaseType type, int size) const {
    return std::make_shared<GlobalArray>(name, type, size);
}

std::shared_ptr<Instruction> InstructionFactory32::createPrintStringFunction() const {
    return std::make_shared<PrintStringFunction>();
}

std::shared_ptr<Instruction> InstructionFactory32::createPrintIntegerFunction() const {
    return std::make_shared<PrintIntegerFunction>();
}

std::shared_ptr<Instruction> InstructionFactory32::createPrintLineFunction() const {
    return std::make_shared<PrintLineFunction>();
}

std::shared_ptr<Instruction> InstructionFactory32::createConcatFunction() const {
    return std::make_shared<ConcatFunction>();
}

std::shared_ptr<Instruction> InstructionFactory32::createMainDeclaration() const {
    return std::make_shared<Main>();
}

std::shared_ptr<Instruction> InstructionFactory32::createDataSection(std::unordered_map<std::string, std::string> pool) const {
    return std::make_shared<DataSection>(pool);
}

std::shared_ptr<Operand> InstructionFactory32::registers(Register reg) const {
   return registerOperands[reg]; 
}
