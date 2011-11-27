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
#include "il/ValueOfOperand.hpp"

using namespace eddic;

std::shared_ptr<Operand> eddic::createImmediateOperand(int value){
    return std::make_shared<ImmediateIntegerOperand>(value);
}

std::shared_ptr<Operand> eddic::createImmediateOperand(const std::string& value){
    return std::make_shared<ImmediateStringOperand>(value);
}

std::shared_ptr<Operand> eddic::createGlobalOperand(const std::string& label, int offset){
    return std::make_shared<GlobalOperand>(label, offset);
}

std::shared_ptr<Operand> eddic::createStackOperand(int offset){
    return std::make_shared<StackOperand>(offset);
}

std::shared_ptr<Operand> eddic::createBaseStackOperand(int offset){
    return std::make_shared<BaseStackOperand>(offset);
}

std::shared_ptr<Operand> eddic::createValueOfOperand(const std::string& location, int offset){
    return std::make_shared<ValueOfOperand>(location, offset);
}
