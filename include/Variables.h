//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLES_H
#define VARIABLES_H

#include <iostream>
#include <map>

#include <commons/Types.h>

class Variable {
	private:
		std::string m_name;
		Type m_type;
		int m_index;
	public:
		Variable(std::string name, Type type, int index) : m_name(name), m_type(type), m_index(index) {}
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
    bool exists(std::string variable) const;
    unsigned int index(std::string variable) const;
    Variable* create(std::string variable, Type type);
	Variable* find(std::string variable);

};

#endif
