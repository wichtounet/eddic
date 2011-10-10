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

using namespace eddic;

std::shared_ptr<Instruction> InstructionFactory::createMOV(){
   return std::shared_ptr<Instruction>(new Instruction()); 
}

std::shared_ptr<Instruction> InstructionFactory::createLabel(const std::string& label){
   return std::shared_ptr<Instruction>(new Label(label)); 
}

std::shared_ptr<Instruction> createCall(const std::string& function){
   return std::shared_ptr<Instruction>(new Call(function)); 
}
