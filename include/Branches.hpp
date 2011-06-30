//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BRANCHES_H
#define BRANCHES_H

#include "Nodes.hpp"

enum BooleanOperator {
	GREATER_OPERATOR,
	LESS_OPERATOR, 
	EQUALS_OPERATOR,
	NOT_EQUALS_OPERATOR, 
	GREATER_EQUALS_OPERATOR,
	LESS_EQUALS_OPERATOR
};

class Condition : public ParseNode {
	private: 
		Value* lhs;
		Value* rhs;
		BooleanOperator operation;

	public:
		Condition(Value* l, Value* r, BooleanOperator o) : lhs(l), rhs(r), operation(o) {}
		
		virtual ~Condition(){
			delete lhs;
			delete rhs;
		}

		virtual void write(ByteCodeFileWriter& writer);
		virtual void checkVariables(Variables& variables) throw (CompilerException);
		virtual void checkStrings(StringPool& pool);
		virtual void optimize();
};

class Else : public ParseNode {
	//Nothing special to do now
};

class If : public ParseNode {
	private: 
		Condition* m_condition;
		Else* m_elseBlock;		

	public:
		If(Condition* condition) : m_condition(condition) {}
		
		virtual ~If(){
			delete m_condition;
			delete m_elseBlock; //Can be null, problem ? 
		}
		
		virtual void write(ByteCodeFileWriter& writer);
		virtual void checkVariables(Variables& variables) throw (CompilerException);
		virtual void checkStrings(StringPool& pool);
		virtual void optimize();

		void setElse(Else* elseBlock){m_elseBlock = elseBlock; }
};

#endif
