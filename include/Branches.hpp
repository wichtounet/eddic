//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BRANCHES_H
#define BRANCHES_H

#include "Nodes.hpp"

class Condition : public ParseNode {
	private: 
		Value* lhs;
		Value* rhs;

	public:
		Condition(Value* l, Value* r) : lhs(l), rhs(r) {}
		
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
	public:
		virtual void write(ByteCodeFileWriter& writer);
		virtual void checkVariables(Variables& variables) throw (CompilerException);
		virtual void checkStrings(StringPool& pool);
		virtual void optimize();
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
