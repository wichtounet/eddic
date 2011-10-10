//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Value.hpp"

using namespace eddic;

Value::Value(std::shared_ptr<Context> context) : ParseNode(context) {}
Value::Value(std::shared_ptr<Context> context, const std::shared_ptr<Token> token) : ParseNode(context, token) {}

Type Value::type() const {
    return m_type;
}

bool Value::isConstant() {
    return false;
}

std::string Value::getStringValue() {
    throw "Not constant";
}

std::string Value::getStringLabel() {
    throw "Not constant";
}

int Value::getStringSize() {
    throw "Not constant";
}

int Value::getIntValue() {
    throw "Not constant";
}
