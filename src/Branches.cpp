//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Branches.hpp"

void Condition::write(ByteCodeFileWriter& writer){
	lhs->write(writer);
	rhs->write(writer);

	switch(operation){
		case GREATER_OPERATOR:
			writer.writeSimpleCall(GREATER_THAN);
			return;
		case LESS_OPERATOR: 
			writer.writeSimpleCall(LESS_THAN);
			return;
		case EQUALS_OPERATOR:
			writer.writeSimpleCall(EQUALS);
			return;
		case NOT_EQUALS_OPERATOR: 
			writer.writeSimpleCall(NOT_EQUALS);
			return;
		case GREATER_EQUALS_OPERATOR:
			writer.writeSimpleCall(GREATER_THAN_EQUALS);
			return;
		case LESS_EQUALS_OPERATOR : 
			writer.writeSimpleCall(LESS_THAN_EQUALS);
			return;
	}
}

void Condition::checkVariables(Variables& variables) throw (CompilerException){
	lhs->checkVariables(variables);
	rhs->checkVariables(variables);

	if(lhs->type() != INT || rhs->type() != INT){
		throw new CompilerException("Can only compare integers");
	}
}

void Condition::checkStrings(StringPool& pool){
	lhs->checkStrings(pool);
	rhs->checkStrings(pool);
}

void Condition::optimize(){
	lhs->optimize();
	rhs->optimize();		
}

void If::write(ByteCodeFileWriter& writer){
	m_condition->write(writer);

	//Make something accessible for others operations
	static int labels = 0;

	int a = labels++;

	writer.writeOneOperandCall(JUMP_IF, a);
	
	ParseNode::write(writer);

	if(m_elseBlock){
		int b = labels++;
	
		writer.writeOneOperandCall(JUMP, b);
	
		writer.writeOneOperandCall(LABEL, a);

		m_elseBlock->write(writer);

		writer.writeOneOperandCall(LABEL, b);
	} else {
		writer.writeOneOperandCall(LABEL, a);
	}
}

void If::checkVariables(Variables& variables) throw (CompilerException){
	m_condition->checkVariables(variables);

	if(m_elseBlock){
		m_elseBlock->checkVariables(variables);
	}

	ParseNode::checkVariables(variables);	
}

void If::checkStrings(StringPool& pool){
	m_condition->checkStrings(pool);

	if(m_elseBlock){
		m_elseBlock->checkStrings(pool);
	}

	ParseNode::checkStrings(pool);	
}

void If::optimize(){
	ParseNode::optimize();
}
