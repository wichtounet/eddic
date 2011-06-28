//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NODES_H
#define NODES_H

#include <string>

#include <commons/Types.hpp>

#include "ParseNode.hpp"

class Program : public ParseNode {
	//Nothing
};

class Value : public ParseNode {
	protected:
		Type m_type;
	public:
		Type type(){return m_type;};
		virtual bool isConstant();
		virtual std::string getStringValue();
		virtual int getIntValue();
};

class Declaration : public ParseNode {
	private: 
		Type m_type;
		std::string m_variable;
		int m_index;
		Value* value;

	public:
		Declaration(Type type, std::string variable, Value* v) : m_type(type), m_variable(variable), value(v) {};
		~Declaration(){delete value;}

		void checkVariables(Variables& variables) throw (CompilerException);
		void write(ByteCodeFileWriter& writer);	
		int index(){return m_index;}
};

class Print : public ParseNode {
	private: 
		Type m_type;
		Value* value;

	public:
		Print(Value* v) : value(v)  {};
		~Print(){delete value;}
		
		void write(ByteCodeFileWriter& writer);	
};

class Assignment : public ParseNode {
	private: 
		std::string m_variable;
		int m_index;
		Value* value;
	
	public:
		Assignment(std::string variable, Value* v) : m_variable(variable), value(v) {};
		~Assignment(){delete value;}
		
		void checkVariables(Variables& variables) throw (CompilerException);
		void write(ByteCodeFileWriter& writer);	
};

class Litteral : public Value {
	private:
		std::string m_litteral;
		int m_index;
	public:
		Litteral(std::string litteral) : m_litteral(litteral) {m_type = STRING;};
		void checkStrings(StringPool& pool);
		void write(ByteCodeFileWriter& writer);
		bool isConstant();
		std::string getStringValue();
};

class Integer : public Value {
	private:
		int m_value;
	public:
		Integer(int value) : m_value(value) {m_type = INT;};
		void write(ByteCodeFileWriter& writer);	
		bool isConstant();
		int getIntValue();
};

class VariableValue : public Value {
	private: 
		std::string m_variable;
		int m_index;
	public:
		VariableValue(std::string variable) : m_variable(variable) {};
		void checkVariables(Variables& variables) throw (CompilerException);
		void write(ByteCodeFileWriter& writer);	
		bool isConstant();
};

class Condition : public ParseNode {
	private: 
		Value* lhs;
		Value* rhs;

	public:
		virtual void write(ByteCodeFileWriter& writer);
		virtual void checkVariables(Variables& variables) throw (CompilerException);
		virtual void checkStrings(StringPool& pool);
		virtual void optimize();
};

class Else;

class If : public ParseNode {
	private: 
		Condition* m_condition;
		Else* m_elseBlock;		

	public:
		If(Condition* condition) : m_condition(condition) {}
		virtual ~If();
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

#endif
