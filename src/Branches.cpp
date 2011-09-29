//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Branches.hpp"
#include "AssemblyFileWriter.hpp"
#include "Context.hpp"

using namespace eddic;

using std::string;

void writeConditionOperands(AssemblyFileWriter& writer, std::shared_ptr<Condition> condition) {
    condition->lhs()->write(writer);
    condition->rhs()->write(writer);

    writer.stream() << "movl 4(%esp), %eax" << std::endl;
    writer.stream() << "movl (%esp), %ebx" << std::endl;
    writer.stream() << "addl $8, %esp" << std::endl;
}

void eddic::writeJumpIfNot(AssemblyFileWriter& writer, std::shared_ptr<Condition> condition, string label, int labelIndex) {
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

void If::checkVariables() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkVariables();
        m_condition->rhs()->checkVariables();

        if (m_condition->lhs()->type() != INT || m_condition->rhs()->type() != INT) {
            throw CompilerException("Can only compare integers", token());
        }
    }

    if (m_elseBlock) {
        m_elseBlock->checkVariables();
    }

    ParseNode::checkVariables();

    for (std::vector<std::shared_ptr<ElseIf>>::iterator it = elseIfs.begin(); it != elseIfs.end(); ++it) {
        (*it)->checkVariables();
    }
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

    for (std::vector<std::shared_ptr<ElseIf>>::iterator it = elseIfs.begin(); it != elseIfs.end(); ++it) {
        (*it)->checkStrings(pool);
    }
}

void If::optimize() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->optimize();
        m_condition->rhs()->optimize();
    }

    for (std::vector<std::shared_ptr<ElseIf>>::iterator it = elseIfs.begin(); it != elseIfs.end(); ++it) {
        (*it)->optimize();
    }

    ParseNode::optimize();
}

void ElseIf::checkVariables() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkVariables();
        m_condition->rhs()->checkVariables();

        if (m_condition->lhs()->type() != INT || m_condition->rhs()->type() != INT) {
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
