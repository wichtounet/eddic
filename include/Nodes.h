//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef NODES_H
#define NODES_H

#include <string>

#include <commons/Types.h>

#include "ParseNode.h"

class Program : public ParseNode {
	//Nothing
};

class Declaration : public ParseNode {
	private: 
		Type m_type;
		std::string m_variable;
		int m_index;
	public:
		Declaration(Type type, std::string variable) : m_type(type), m_variable(variable) {};
		void checkVariables(Variables& variables) throw (CompilerException);
		void write(ByteCodeFileWriter& writer);	
		int index(){return m_index;}
};

class Print : public ParseNode {
	private: 
		Type m_type;
	public:
		Print(){};
		void write(ByteCodeFileWriter& writer);	
};

class Assignment : public ParseNode {
	private: 
		std::string m_variable;
		int m_index;
	public:
		Assignment(std::string variable) : m_variable(variable) {};
		void checkVariables(Variables& variables) throw (CompilerException);
		void write(ByteCodeFileWriter& writer);	
};

class Value : public ParseNode {
	protected:
		Type m_type;
	public:
		Type type(){return m_type;};
};

class Litteral : public Value {
	private:
		std::string m_litteral;
		int m_index;
	public:
		Litteral(std::string litteral) : m_litteral(litteral) {m_type = STRING;};
		void checkStrings(StringPool& pool);
		void write(ByteCodeFileWriter& writer);	
};

class Integer : public Value {
	private:
		int m_value;
	public:
		Integer(int value) : m_value(value) {m_type = INT;};
		void write(ByteCodeFileWriter& writer);	
};

class VariableValue : public Value {
	private: 
		std::string m_variable;
		int m_index;
	public:
		VariableValue(std::string variable) : m_variable(variable) {};
		void checkVariables(Variables& variables) throw (CompilerException);
		void write(ByteCodeFileWriter& writer);	
};

#endif
