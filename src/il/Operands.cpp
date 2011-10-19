//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "il/Operands.hpp"

#include "il/ImmediateOperand.hpp"
#include "il/RegisterOperand.hpp"
#include "il/GlobalOperand.hpp"
#include "il/StackOperand.hpp"
#include "il/BaseStackOperand.hpp"

using namespace eddic;

std::shared_ptr<Operand> eddic::createImmediateOperand(int value){
    return std::shared_ptr<Operand>(new ImmediateOperand(value));
}

std::shared_ptr<Operand> eddic::createImmediateOperand(std::string value){
    return std::shared_ptr<Operand>(new ImmediateOperand(value));
}

std::shared_ptr<Operand> eddic::createRegisterOperand(std::string reg){
    return std::shared_ptr<Operand>(new RegisterOperand(reg));
}

std::shared_ptr<Operand> eddic::createGlobalOperand(std::string label){
    return std::shared_ptr<Operand>(new GlobalOperand(label));
}

std::shared_ptr<Operand> eddic::createGlobalOperand(std::string label, int offset){
    return std::shared_ptr<Operand>(new GlobalOperand(label, offset));
}

std::shared_ptr<Operand> eddic::createStackOperand(int offset){
    return std::shared_ptr<Operand>(new StackOperand(offset));
}

std::shared_ptr<Operand> eddic::createBaseStackOperand(int offset){
    return std::shared_ptr<Operand>(new BaseStackOperand(offset));
}
