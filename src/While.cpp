//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "While.hpp"
#include "AssemblyFileWriter.hpp"
#include "Context.hpp"

#include "If.hpp"
#include "Condition.hpp"

#include "Value.hpp"

using namespace eddic;

void While::write(AssemblyFileWriter& writer) {
    //Make something accessible for others operations
    static int labels = 0;

    int startLabel = labels++;
    int endLabel = labels++;

    writer.stream() << "WL" << startLabel << ":" << std::endl;

    writeJumpIfNot(writer, m_condition, "WL", endLabel);

    ParseNode::write(writer);

    writer.stream() << "jmp WL" << startLabel << std::endl;

    writer.stream() << "WL" << endLabel << ":" << std::endl;
}

void While::checkVariables() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkVariables();
        m_condition->rhs()->checkVariables();

        if (m_condition->lhs()->type() != Type::INT || m_condition->rhs()->type() != Type::INT) {
            throw CompilerException("Can only compare integers", token());
        }
    }

    ParseNode::checkVariables();
}

void While::checkStrings(StringPool& pool) {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkStrings(pool);
        m_condition->rhs()->checkStrings(pool);
    }

    ParseNode::checkStrings(pool);
}

void While::optimize() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->optimize();
        m_condition->rhs()->optimize();
    }

    ParseNode::optimize();
}
