//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "For.hpp"
#include "AssemblyFileWriter.hpp"
#include "Context.hpp"

#include "If.hpp"
#include "Condition.hpp"

#include "Value.hpp"
#include "SpiritToken.hpp"

using namespace eddic;

For::For(std::shared_ptr<Context> context, const Tok& token, std::shared_ptr<ParseNode> start, std::shared_ptr<Condition> condition, std::shared_ptr<ParseNode> iter) : ParseNode(context, token), m_start(start), m_iter(iter), m_condition(condition) {}

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
