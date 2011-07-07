//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BRANCHES_H
#define BRANCHES_H

#include <vector>

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
	//Do nothing
};

class BooleanComparison : public Condition {
	private: 
		Value* lhs;
		Value* rhs;
		BooleanOperator operation;

	public:
		BooleanComparison(Value* l, Value* r, BooleanOperator o) : lhs(l), rhs(r), operation(o) {}
		
		virtual ~BooleanComparison(){
			delete lhs;
			delete rhs;
		}

		virtual void write(ByteCodeFileWriter& writer);
		virtual void checkVariables(Variables& variables) throw (CompilerException);
		virtual void checkStrings(StringPool& pool);
		virtual void optimize();
};

class True : public Condition {
	virtual void write(ByteCodeFileWriter& writer);
};

class False : public Condition {
	virtual void write(ByteCodeFileWriter& writer);
};

class Else : public ParseNode {
	//Nothing special to do now
};

class ElseIf : public ParseNode {
	private: 
		Condition* m_condition;

	public:
		ElseIf(Condition* condition) : m_condition(condition) {}
		
		virtual ~ElseIf(){
			delete m_condition;
		}
		
		virtual void checkVariables(Variables& variables) throw (CompilerException);
		virtual void checkStrings(StringPool& pool);
		virtual void optimize();
		
		Condition* condition(){ return m_condition; }
};

class If : public ParseNode {
	private: 
		Condition* m_condition;
		Else* m_elseBlock;		
		std::vector<ElseIf*> elseIfs;

	public:
		If(Condition* condition) : m_condition(condition), m_elseBlock(NULL) {}
		
		virtual ~If();
		
		virtual void write(ByteCodeFileWriter& writer);
		virtual void checkVariables(Variables& variables) throw (CompilerException);
		virtual void checkStrings(StringPool& pool);
		virtual void optimize();

		void setElse(Else* elseBlock){m_elseBlock = elseBlock; }
		void addElseIf(ElseIf* elseIf){elseIfs.push_back(elseIf);}
};

#endif
