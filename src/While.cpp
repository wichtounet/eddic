//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "While.hpp"
#include "Context.hpp"
#include "CompilerException.hpp"

#include "If.hpp"
#include "Condition.hpp"

#include "Value.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Labels.hpp"

using namespace eddic;

While::While(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Condition> condition) : ParseNode(context, token), m_condition(condition) {}

std::shared_ptr<Condition> While::condition() {
    return m_condition;
}

void While::writeIL(IntermediateProgram& program) {
    //Make something accessible for others operations
    static int labels = 0;

    int startLabel = labels++;
    int endLabel = labels++;

    program.addInstruction(program.factory().createLabel(label("WL", startLabel)));

    writeILJumpIfNot(program, m_condition, "WL", endLabel);

    ParseNode::writeIL(program);

    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label("WL", startLabel)));

    program.addInstruction(program.factory().createLabel(label("WL", endLabel)));
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
