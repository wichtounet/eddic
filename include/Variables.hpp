//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLES_H
#define VARIABLES_H

#include <string>
#include <map>

#include "Types.hpp"

#include "ByteCodeFileWriter.hpp"

namespace eddic {

class Variable {
	private:
		std::string m_name;
		Type m_type;
		int m_index;
	public:
		Variable(const std::string& name, Type type, int index) : m_name(name), m_type(type), m_index(index) {}
		std::string name(){return m_name;}
		int index(){return m_index;}
		Type type(){return m_type;}
};

class Variables {
	private:
		std::map<std::string, Variable*> variables;
		unsigned int currentVariable;
	public:
		Variables(){currentVariable = 0;};
		~Variables();
		bool exists(const std::string& variable) const;
		unsigned int index(const std::string& variable) const;
		Variable* create(const std::string& variable, Type type);
		Variable* find(const std::string& variable);
		void write(ByteCodeFileWriter& writer);
};

} //end of eddic

#endif
