//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cctype>

#include "Program.h"

void Program::addInstruction(Instruction* instruction){
	instructions.push_back(instruction);
}

std::vector<Instruction*>::iterator Program::begin(){
	return instructions.begin();
}

std::vector<Instruction*>::iterator Program::end(){
	return instructions.end();
}
