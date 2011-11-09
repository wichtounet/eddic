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
    return std::make_shared<ImmediateIntegerOperand>(value);
}

std::shared_ptr<Operand> eddic::createImmediateOperand(std::string value){
    return std::make_shared<ImmediateStringOperand>(value);
}

std::shared_ptr<Operand> eddic::createRegisterOperand(std::string reg){
    return std::make_shared<RegisterOperand>(reg);
}

std::shared_ptr<Operand> eddic::createGlobalOperand(std::string label){
    return std::make_shared<GlobalOperand>(label);
}

std::shared_ptr<Operand> eddic::createGlobalOperand(std::string label, int offset){
    return std::make_shared<GlobalOperand>(label, offset);
}

std::shared_ptr<Operand> eddic::createStackOperand(int offset){
    return std::make_shared<StackOperand>(offset);
}

std::shared_ptr<Operand> eddic::createBaseStackOperand(int offset){
    return std::make_shared<BaseStackOperand>(offset);
}
