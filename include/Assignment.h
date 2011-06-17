//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include "Instruction.h"
#include "Value.h"

class Assignment : public Instruction {
	private:
		std::string m_variable;
		Value* m_value;
	public:
		Assignment(std::string variable, Value* value) : m_variable(variable), m_value(value) {};
		std::string variable(){return m_variable;};
		Value* value(){return m_value;};
};

#endif
