//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VALUE_H
#define VALUE_H

#include "Instruction.h"
#include "Value.h"

class Value {
	//Nothing
};

class Litteral : public Value {
	private:
		std::string m_litteral;
	public:
		Litteral(std::string litteral) : m_litteral(litteral) {}
		std::string value(){return m_litteral;};
};

class Integer : public Value {
	private:
		int m_integer;
	public:
		Integer(int integer) : m_integer(integer) {}
		int value(){return m_integer;};
};

class Variable : public Value {
	private:
		std::string m_variable;
	public:
		Variable(std::string variable) : m_variable(variable) {}
		std::string value(){return m_variable;};
};

#endif
