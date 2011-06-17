//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DECLARATION_H
#define DECLARATION_H

#include "commons/Types.h"

#include "Instruction.h"

class Declaration : public Instruction {
	private:
		Type m_type;
		std::string m_variable;
		Value* m_value;
	public:
		Declaration(Type type, std::string variable, Value* value) : m_type(type), m_variable(variable), m_value(value) {};
		Type type(){return m_type;};
		Value* value(){return m_value;};
};

#endif
