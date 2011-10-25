//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include "If.hpp"
#include "AssemblyFileWriter.hpp"
#include "Context.hpp"
#include "CompilerException.hpp"
#include "Utils.hpp"
#include "Else.hpp"
#include "ElseIf.hpp"
#include "Condition.hpp"
#include "Value.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"
#include "il/Labels.hpp"

using namespace eddic;

using std::string;

If::If(std::shared_ptr<Context> context, const Tok& token, std::shared_ptr<Condition> condition) : ParseNode(context, token), m_condition(condition) {}

void If::setElse(std::shared_ptr<Else> elseBlock) {
    m_elseBlock = elseBlock;
}

void If::addElseIf(std::shared_ptr<ElseIf> elseIf) {
    elseIfs.push_back(elseIf);
}

std::shared_ptr<Condition> If::condition() {
    return m_condition;
}

void eddic::writeILJumpIfNot(IntermediateProgram& program, std::shared_ptr<Condition> condition, const string& label, int labelIndex) {
    if (!condition->isOperator()) {
        //No need to jump if true
        if (condition->condition() == FALSE_VALUE) {
            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label(label, labelIndex)));
        }
    } else {
        condition->lhs()->assignTo(createRegisterOperand("eax"), program);
        condition->rhs()->assignTo(createRegisterOperand("ebx"), program);

        program.addInstruction(program.factory().createCompare(createRegisterOperand("ebx"), createRegisterOperand("eax")));

        switch (condition->condition()) {
            case GREATER_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::LESS_EQUALS, eddic::label(label, labelIndex)));

                break;
            case GREATER_EQUALS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::LESS, eddic::label(label, labelIndex)));

                break;
            case LESS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::GREATER_EQUALS, eddic::label(label, labelIndex)));

                break;
            case LESS_EQUALS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::GREATER, eddic::label(label, labelIndex)));

                break;
            case EQUALS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::NOT_EQUALS, eddic::label(label, labelIndex)));

                break;
            case NOT_EQUALS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::EQUALS, eddic::label(label, labelIndex)));

                break;
            //TODO default:
                //TODO throw CompilerException("The condition must be managed using not-operator");
        }
    }
}

void If::writeIL(IntermediateProgram& program){
    //Make something accessible for others operations
    static int labels = 0;

    if (elseIfs.empty()) {
        int a = labels++;

        writeILJumpIfNot(program, m_condition, "L", a);

        ParseNode::writeIL(program);

        if (m_elseBlock) {
            int b = labels++;

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label("L", b)));

            program.addInstruction(program.factory().createLabel(eddic::label("L", a)));

            m_elseBlock->writeIL(program);

            program.addInstruction(program.factory().createLabel(eddic::label("L", b)));
        } else {
            program.addInstruction(program.factory().createLabel(eddic::label("L", a)));
        }
    } else {
        int end = labels++;
        int next = labels++;

        writeILJumpIfNot(program, m_condition, "L", next);

        ParseNode::writeIL(program);

        program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label("L", end)));

        for (std::vector<std::shared_ptr<ElseIf>>::size_type i = 0; i < elseIfs.size(); ++i) {
            std::shared_ptr<ElseIf> elseIf = elseIfs[i];

            program.addInstruction(program.factory().createLabel(eddic::label("L", next)));

            //Last elseif
            if (i == elseIfs.size() - 1) {
                if (m_elseBlock) {
                    next = labels++;
                } else {
                    next = end;
                }
            } else {
                next = labels++;
            }

            writeILJumpIfNot(program, elseIf->condition(), "L", next);

            elseIf->writeIL(program);

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, eddic::label("L", end)));
        }

        if (m_elseBlock) {
            program.addInstruction(program.factory().createLabel(eddic::label("L", next)));

            m_elseBlock->writeIL(program);
        }

        program.addInstruction(program.factory().createLabel(eddic::label("L", end)));
    }
}

void If::checkVariables() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkVariables();
        m_condition->rhs()->checkVariables();

        if (m_condition->lhs()->type() != Type::INT || m_condition->rhs()->type() != Type::INT) {
            throw CompilerException("Can only compare integers", token());
        }
    }

    if (m_elseBlock) {
        m_elseBlock->checkVariables();
    }

    ParseNode::checkVariables();
    
    for_each(elseIfs.begin(), elseIfs.end(), [](std::shared_ptr<ElseIf> e){ e->checkVariables(); });
}

void If::checkStrings(StringPool& pool) {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkStrings(pool);
        m_condition->rhs()->checkStrings(pool);
    }

    if (m_elseBlock) {
        m_elseBlock->checkStrings(pool);
    }

    ParseNode::checkStrings(pool);
    
    for_each(elseIfs.begin(), elseIfs.end(), [&](std::shared_ptr<ElseIf> e){ e->checkStrings(pool); });
}

void If::optimize() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->optimize();
        m_condition->rhs()->optimize();
    }
    
    for_each(elseIfs.begin(), elseIfs.end(), [](std::shared_ptr<ElseIf> e){ e->optimize(); });

    ParseNode::optimize();
}
