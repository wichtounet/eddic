//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <algorithm>

#include "AssemblyFileWriter.hpp"
#include "il/IntermediateProgram.hpp"
#include "il/Operands.hpp"
#include "Context.hpp"
#include "Utils.hpp"

#include "If.hpp"
#include "Else.hpp"
#include "ElseIf.hpp"
#include "Condition.hpp"

#include "Value.hpp"

using namespace eddic;

using std::string;

If::If(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<Condition> condition) : ParseNode(context, token), m_condition(condition) {}

void If::setElse(std::shared_ptr<Else> elseBlock) {
    m_elseBlock = elseBlock;
}

void If::addElseIf(std::shared_ptr<ElseIf> elseIf) {
    elseIfs.push_back(elseIf);
}

std::shared_ptr<Condition> If::condition() {
    return m_condition;
}

void writeConditionOperands(AssemblyFileWriter& writer, std::shared_ptr<Condition> condition) {
    condition->lhs()->write(writer);
    condition->rhs()->write(writer);

    writer.stream() << "movl 4(%esp), %eax" << std::endl;
    writer.stream() << "movl (%esp), %ebx" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;
}

void eddic::writeJumpIfNot(AssemblyFileWriter& writer, std::shared_ptr<Condition> condition, const string& label, int labelIndex) {
    if (!condition->isOperator()) {
        //No need to jump if true
        if (condition->condition() == FALSE_VALUE) {
            writer.stream() << "jmp " << label << labelIndex << std::endl;
        }
    } else {
        writeConditionOperands(writer, condition);

        writer.stream() << "cmpl %ebx, %eax" << std::endl;

        switch (condition->condition()) {
            case GREATER_OPERATOR:
                writer.stream() << "jle " << label << labelIndex << std::endl;

                break;
            case GREATER_EQUALS_OPERATOR:
                writer.stream() << "jl " << label << labelIndex << std::endl;

                break;
            case LESS_OPERATOR:
                writer.stream() << "jge " << label << labelIndex << std::endl;

                break;
            case LESS_EQUALS_OPERATOR:
                writer.stream() << "jg " << label << labelIndex << std::endl;

                break;
            case EQUALS_OPERATOR:
                writer.stream() << "jne " << label << labelIndex << std::endl;

                break;
            case NOT_EQUALS_OPERATOR:
                writer.stream() << "je " << label << labelIndex << std::endl;

                break;
            default:
                throw CompilerException("The condition must be managed using not-operator");
        }
    }
}

void eddic::writeILJumpIfNot(IntermediateProgram& program, std::shared_ptr<Condition> condition, const string& label, int labelIndex) {
    if (!condition->isOperator()) {
        //No need to jump if true
        if (condition->condition() == FALSE_VALUE) {
            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label + toString(labelIndex)));
        }
    } else {
        condition->lhs()->assignTo(createRegisterOperand("eax"), program);
        condition->rhs()->assignTo(createRegisterOperand("ebx"), program);

        program.addInstruction(program.factory().createCompare(createRegisterOperand("ebx"), createRegisterOperand("eax")));

        switch (condition->condition()) {
            case GREATER_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::LESS_EQUALS, label + toString(labelIndex)));

                break;
            case GREATER_EQUALS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::LESS, label + toString(labelIndex)));

                break;
            case LESS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::GREATER_EQUALS, label + toString(labelIndex)));

                break;
            case LESS_EQUALS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::GREATER, label + toString(labelIndex)));

                break;
            case EQUALS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::NOT_EQUALS, label + toString(labelIndex)));

                break;
            case NOT_EQUALS_OPERATOR:
                program.addInstruction(program.factory().createJump(JumpCondition::EQUALS, label + toString(labelIndex)));

                break;
            default:
                throw CompilerException("The condition must be managed using not-operator");
        }
    }
}

void If::write(AssemblyFileWriter& writer) {
    //Make something accessible for others operations
    static int labels = 0;

    if (elseIfs.empty()) {
        int a = labels++;

        writeJumpIfNot(writer, m_condition, "L", a);

        ParseNode::write(writer);

        if (m_elseBlock) {
            int b = labels++;

            writer.stream() << "jmp L" << b << std::endl;

            writer.stream() << "L" << a << ":" << std::endl;

            m_elseBlock->write(writer);

            writer.stream() << "L" << b << ":" << std::endl;
        } else {
            writer.stream() << "L" << a << ":" << std::endl;
        }
    } else {
        int end = labels++;
        int next = labels++;

        writeJumpIfNot(writer, m_condition, "L", next);

        ParseNode::write(writer);

        writer.stream() << "jmp L" << end << std::endl;

        for (std::vector<std::shared_ptr<ElseIf>>::size_type i = 0; i < elseIfs.size(); ++i) {
            std::shared_ptr<ElseIf> elseIf = elseIfs[i];

            writer.stream() << "L" << next << ":" << std::endl;

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

            writeJumpIfNot(writer, elseIf->condition(), "L", next);

            elseIf->write(writer);

            writer.stream() << "jmp L" << end << std::endl;
        }

        if (m_elseBlock) {
            writer.stream() << "L" << next << ":" << std::endl;

            m_elseBlock->write(writer);
        }

        writer.stream() << "L" << end << ":" << std::endl;
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

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, "L" + toString(b)));

            program.addInstruction(program.factory().createLabel("L" + toString(a)));

            m_elseBlock->writeIL(program);

            program.addInstruction(program.factory().createLabel("L" + toString(b)));
        } else {
            program.addInstruction(program.factory().createLabel("L" + toString(a)));
        }
    } else {
        int end = labels++;
        int next = labels++;

        writeILJumpIfNot(program, m_condition, "L", next);

        ParseNode::writeIL(program);

        program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, "L" + toString(end)));

        for (std::vector<std::shared_ptr<ElseIf>>::size_type i = 0; i < elseIfs.size(); ++i) {
            std::shared_ptr<ElseIf> elseIf = elseIfs[i];

            program.addInstruction(program.factory().createLabel("L" + toString(next)));

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

            program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, "L" + toString(end)));
        }

        if (m_elseBlock) {
            program.addInstruction(program.factory().createLabel("L" + toString(next)));

            m_elseBlock->writeIL(program);
        }

        program.addInstruction(program.factory().createLabel("L" + toString(end)));
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
