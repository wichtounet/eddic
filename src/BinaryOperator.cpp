//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "BinaryOperator.hpp"
#include "Options.hpp"
#include "AssemblyFileWriter.hpp"
#include "Context.hpp"
#include "Integer.hpp"

#include <cassert>

using std::string;

using namespace eddic;

BinaryOperator::BinaryOperator(std::shared_ptr<Context> context, const Tok token, std::shared_ptr<Value> l, std::shared_ptr<Value> r) : Value(context, token), lhs(l), rhs(r) {}

void BinaryOperator::checkVariables() {
    lhs->checkVariables();
    rhs->checkVariables();

    m_type = checkTypes(lhs->type(), rhs->type());
}

void BinaryOperator::checkStrings(StringPool& pool) {
    lhs->checkStrings(pool);
    rhs->checkStrings(pool);
}

Type BinaryOperator::checkTypes(Type left, Type right) {
    if (left != right || left != Type::INT) {
        throw CompilerException("Can only compute two integers", token());
    }

    return left;
}

bool BinaryOperator::isConstant() {
    return lhs->isConstant() && rhs->isConstant();
}

int BinaryOperator::getIntValue() {
    if (type() != Type::INT) {
        throw "Invalid type";
    }

    if (!isConstant()) {
        throw "Not a constant";
    }

    return compute(lhs->getIntValue(), rhs->getIntValue());
}

string BinaryOperator::getStringValue() {
    if (type() != Type::STRING) {
        throw "Invalid type";
    }

    if (!isConstant()) {
        throw "Not a constant";
    }

    return compute(lhs->getStringValue(), rhs->getStringValue());
}

int BinaryOperator::compute(int, int) {
    throw "Invalid type";
}

string BinaryOperator::compute(const std::string&, const std::string&) {
    throw "Invalid type";
}

void BinaryOperator::optimize() {
    if (isConstant()) {
        if (Options::isSet(BooleanOption::OPTIMIZE_INTEGERS)) {
            std::shared_ptr<Value> value(new Integer(context(), lhs->token(), getIntValue()));

            parent.lock()->replace(shared_from_this(), value);
        }
    }

    lhs->optimize();
    rhs->optimize();
}
