//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include "AssemblyFileWriter.hpp"
#include "Context.hpp"

#include "ElseIf.hpp"
#include "Condition.hpp"

#include "Value.hpp"

using namespace eddic;

using std::string;

void ElseIf::checkVariables() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkVariables();
        m_condition->rhs()->checkVariables();

        if (m_condition->lhs()->type() != Type::INT || m_condition->rhs()->type() != Type::INT) {
            throw CompilerException("Can only compare integers", token());
        }
    }

    ParseNode::checkVariables();
}

void ElseIf::checkStrings(StringPool& pool) {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkStrings(pool);
        m_condition->rhs()->checkStrings(pool);
    }

    ParseNode::checkStrings(pool);
}

void ElseIf::optimize() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->optimize();
        m_condition->rhs()->optimize();
    }

    ParseNode::optimize();
}
