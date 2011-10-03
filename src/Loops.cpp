//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Loops.hpp"
#include "AssemblyFileWriter.hpp"
#include "Context.hpp"
#include "Branches.hpp"

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

        if (m_condition->lhs()->type() != INT || m_condition->rhs()->type() != INT) {
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

void For::write(AssemblyFileWriter& writer){
    if(m_start){
        m_start->write(writer);
    }

    static int labels = -1;

    ++labels;

    writer.stream() << "start_for" << labels << ":" << std::endl;

    if(m_condition){
        writeJumpIfNot(writer, m_condition, "end_for", labels);
    }

    ParseNode::write(writer);

    if(m_iter){
        m_iter->write(writer);
    }

    writer.stream() << "jmp start_for" << labels << std::endl;

    writer.stream() << "end_for" << labels << ":" << std::endl;
}

void For::checkVariables(){
    if(m_start){
        m_start->checkVariables();
    }
    
    if(m_condition){
        m_condition->lhs()->checkVariables();
        m_condition->rhs()->checkVariables();
    }

    if(m_iter){
        m_iter->checkVariables();
    }

    ParseNode::checkVariables();
}

void For::checkStrings(StringPool& pool){
    if(m_start){
        m_start->checkStrings(pool);
    }
    
    if(m_condition){
        m_condition->lhs()->checkStrings(pool);
        m_condition->rhs()->checkStrings(pool);
    }

    if(m_iter){
        m_iter->checkStrings(pool);
    }

    ParseNode::checkStrings(pool);
}

void For::optimize(){
    if(m_condition){
        m_condition->lhs()->optimize();
        m_condition->rhs()->optimize();
    }

    if(m_start){
        m_start->optimize();
    }
    
    if(m_iter){
        m_iter->optimize();
    }

    ParseNode::optimize();
}

void Foreach::write(AssemblyFileWriter& writer){
    //TODO Review
}

void Foreach::checkVariables(){
    m_from->checkVariables();
    m_to->checkVariables();

    ParseNode::checkVariables();
}

void Foreach::checkStrings(StringPool& pool){
    m_from->checkStrings(pool);
    m_to->checkStrings(pool);

    ParseNode::checkStrings(pool);
}

void Foreach::optimize(){
    m_from->optimize();
    m_to->optimize();

    ParseNode::optimize();
}
