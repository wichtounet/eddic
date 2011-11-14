//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPERANDS_H
#define OPERANDS_H

#include <memory>
#include <string>

#include "Operand.hpp"
#include "il/ValueOfOperand.hpp"

namespace eddic {

//TODO Put default values for the offset

std::shared_ptr<Operand> createImmediateOperand(int value);
std::shared_ptr<Operand> createImmediateOperand(std::string value);
std::shared_ptr<Operand> createGlobalOperand(std::string label);
std::shared_ptr<Operand> createGlobalOperand(std::string label, int offset);
std::shared_ptr<Operand> createStackOperand(int offset);
std::shared_ptr<Operand> createBaseStackOperand(int offset);

} //end of eddic

#endif
