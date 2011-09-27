//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <vector>
#include <string>

#include "Nodes.hpp"

namespace eddic {

template<typename T>
std::string mangle(std::string functionName, std::vector<T*> typed){
    std::string ss;

    ss += "_F";
    ss += functionName.length();
    ss += functionName;

    typename std::vector<T*>::const_iterator it = typed.begin();

    for( ; it != typed.end(); ++it){
        ss += mangle((*it)->type());
    }

    return ss;
}

std::string mangle(Type type);

class MainDeclaration : public ParseNode {
   public:
        MainDeclaration(Context* context) : ParseNode(context) {};

        void write(AssemblyFileWriter& writer);
};

class Parameter {
    private:
        std::string m_name;
        Type m_type;
        int m_offset;

    public:
        Parameter(const std::string& name, Type type, int offset) : m_name(name), m_type(type), m_offset(offset) {}

        Type type(){
            return m_type;
        }

        int offset(){
            return m_offset;
        }
};

class Function : public ParseNode {
	private:
		std::string m_name;
        std::vector<Parameter> m_parameters;
        int m_currentPosition;

	public:
		Function(Context* context, const std::string& name) : ParseNode(context), m_name(name), m_currentPosition(0) {}
		
        void write(AssemblyFileWriter& writer);

        std::string name(){
            return m_name;
        }

        void addParameter(std::string name, Type type);
};

class FunctionCall : public ParseNode {
    private:
        std::string m_function;
        std::string m_function_mangled;
        std::vector<Value*> m_values;

    public:
        FunctionCall(Context* context, const std::string& function) : ParseNode(context), m_function(function) {}

        void write(AssemblyFileWriter& writer);
        void checkFunctions(Program& program);

        void addValue(Value* value){
            m_values.push_back(value);
            addLast(value);
        }
};

} //end of eddic

#endif
