//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Branches.hpp"
#include "AssemblyFileWriter.hpp"
#include "Variables.hpp"

using namespace eddic;

If::~If() {
    delete m_condition;
    delete m_elseBlock; //Can be null, problem ?

    for (std::vector<ElseIf*>::iterator it = elseIfs.begin(); it != elseIfs.end(); ++it) {
        delete *it;
    }
}

void writeCondition(AssemblyFileWriter& writer, Condition* condition, int label) {
    if (condition->isOperator()) {
        condition->lhs()->write(writer);
        condition->rhs()->write(writer);

        writer.stream() << "movl 4(%esp), %eax" << std::endl;
        writer.stream() << "movl (%esp), %ebx" << std::endl;
        writer.stream() << "addl $8, %esp" << std::endl;
    }

    switch (condition->condition()) {
        case TRUE_VALUE:
            //No need to jump

            break;
        case FALSE_VALUE:
            writer.stream() << "jmp L" << label << std::endl;

            break;
        case GREATER_OPERATOR:
            writer.stream() << "cmpl %ebx, %eax" << std::endl;
            writer.stream() << "jle L" << label << std::endl;

            break;
        case GREATER_EQUALS_OPERATOR:
            writer.stream() << "cmpl %ebx, %eax" << std::endl;
            writer.stream() << "jl L" << label << std::endl;

            break;
        case LESS_OPERATOR:
            writer.stream() << "cmpl %ebx, %eax" << std::endl;
            writer.stream() << "jge L" << label << std::endl;

            break;
        case LESS_EQUALS_OPERATOR:
            writer.stream() << "cmpl %ebx, %eax" << std::endl;
            writer.stream() << "jg L" << label << std::endl;

            break;
        case EQUALS_OPERATOR:
            writer.stream() << "cmpl %ebx, %eax" << std::endl;
            writer.stream() << "jne L" << label << std::endl;

            break;
        case NOT_EQUALS_OPERATOR:
            writer.stream() << "cmpl %ebx, %eax" << std::endl;
            writer.stream() << "je L" << label << std::endl;

            break;
    }
}

void If::write(AssemblyFileWriter& writer) {
    //Make something accessible for others operations
    static int labels = 0;

    if (elseIfs.empty()) {
        int a = labels++;

        writeCondition(writer, m_condition, a);

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

        writeCondition(writer, m_condition, next);

        ParseNode::write(writer);

        writer.stream() << "jmp L" << end << std::endl;

        for (std::vector<ElseIf*>::size_type i = 0; i < elseIfs.size(); ++i) {
            ElseIf* elseIf = elseIfs[i];

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

            writeCondition(writer, elseIf->condition(), next);

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

void If::checkVariables(Variables& variables) {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkVariables(variables);
        m_condition->rhs()->checkVariables(variables);

        if (m_condition->lhs()->type() != INT || m_condition->rhs()->type() != INT) {
            throw CompilerException("Can only compare integers");
        }
    }

    if (m_elseBlock) {
        m_elseBlock->checkVariables(variables);
    }

    ParseNode::checkVariables(variables);

    for (std::vector<ElseIf*>::iterator it = elseIfs.begin(); it != elseIfs.end(); ++it) {
        (*it)->checkVariables(variables);
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

    for (std::vector<ElseIf*>::iterator it = elseIfs.begin(); it != elseIfs.end(); ++it) {
        (*it)->checkStrings(pool);
    }
}

void If::optimize() {
    if (m_condition->isOperator()) {
        m_condition->lhs()->optimize();
        m_condition->rhs()->optimize();
    }

    for (std::vector<ElseIf*>::iterator it = elseIfs.begin(); it != elseIfs.end(); ++it) {
        (*it)->optimize();
    }

    ParseNode::optimize();
}

void ElseIf::checkVariables(Variables& variables) {
    if (m_condition->isOperator()) {
        m_condition->lhs()->checkVariables(variables);
        m_condition->rhs()->checkVariables(variables);

        if (m_condition->lhs()->type() != INT || m_condition->rhs()->type() != INT) {
            throw CompilerException("Can only compare integers");
        }
    }

    ParseNode::checkVariables(variables);
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
