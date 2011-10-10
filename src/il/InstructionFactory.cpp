//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/InstructionFactory.hpp"
#include "il/Instruction.hpp"

using namespace eddic;

Instruction InstructionFactory::createMOV(){
   return Instruction(); 
}

Instruction InstructionFactory::createLabel(const std::string& label){
   return Instruction(); 
}
