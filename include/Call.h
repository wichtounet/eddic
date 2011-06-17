//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CALL_H
#define CALL_H

#include "Instruction.h"
#include "Value.h"

class Call : public Instruction {
	private:
		std::string m_operation;
		Value* m_value;
	public:
		Call(std::string operation, Value* value) : m_operation(operation), m_value(value) {};
		std::string operation(){return m_operation;};
		Value* value(){return m_value;};
};

#endif
