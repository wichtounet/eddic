//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "For.hpp"
#include "Context.hpp"

#include "If.hpp"
#include "Condition.hpp"

#include "Value.hpp"
#include "il/Labels.hpp"

#include "il/IntermediateProgram.hpp"
#include "il/Labels.hpp"

using namespace eddic;

For::For(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, std::shared_ptr<ParseNode> start, std::shared_ptr<Condition> condition, std::shared_ptr<ParseNode> iter) : ParseNode(context, token), m_start(start), m_iter(iter), m_condition(condition) {}

void For::writeIL(IntermediateProgram& program){
    if(m_start){
        m_start->writeIL(program);
    }

    static int labels = -1;

    ++labels;

    program.addInstruction(program.factory().createLabel(label("start_for", labels)));

    if(m_condition){
        writeILJumpIfNot(program, m_condition, "end_for", labels);
    }

    ParseNode::writeIL(program);

    if(m_iter){
        m_iter->writeIL(program);
    }

    program.addInstruction(program.factory().createJump(JumpCondition::ALWAYS, label("start_for", labels)));

    program.addInstruction(program.factory().createLabel(label("end_for", labels)));
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
