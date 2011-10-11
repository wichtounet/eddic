//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/InstructionFactory.hpp"
#include "il/Instruction.hpp"
#include "il/Label.hpp"
#include "il/Call.hpp"
#include "il/Jump.hpp"
#include "il/FunctionDeclaration.hpp"
#include "il/FunctionExit.hpp"
#include "il/Operand.hpp"
#include "il/Move.hpp"

using namespace eddic;

std::shared_ptr<Instruction> InstructionFactory::createMove(std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs){
   return std::shared_ptr<Instruction>(new Move(lhs, rhs)); 
}

std::shared_ptr<Instruction> InstructionFactory::createLabel(const std::string& label){
   return std::shared_ptr<Instruction>(new Label(label)); 
}

std::shared_ptr<Instruction> InstructionFactory::createJump(const std::string& label){
   return std::shared_ptr<Instruction>(new Jump(label)); 
}

std::shared_ptr<Instruction> InstructionFactory::createCall(const std::string& function){
   return std::shared_ptr<Instruction>(new Call(function)); 
}

std::shared_ptr<Instruction> InstructionFactory::createFunctionDeclaration(const std::string& function, int size){
   return std::shared_ptr<Instruction>(new FunctionDeclaration(function, size)); 
}

std::shared_ptr<Instruction> InstructionFactory::createFunctionExit(int size){
   return std::shared_ptr<Instruction>(new FunctionExit(size)); 
}

std::shared_ptr<Instruction> createMath(Operation operation, std::shared_ptr<Operand> lhs, std::shared_ptr<Operand> rhs){
   return std::shared_ptr<Instruction>(new Math(operation, lhs, rhs)); 
}
