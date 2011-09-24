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

class MainDeclaration : public ParseNode {
   public:
        MainDeclaration(Context* context) : ParseNode(context) {};

        void write(AssemblyFileWriter& writer);
};

class Function : public ParseNode {
	private:
		std::string m_name;

	public:
		Function(Context* context, const Token* token, const std::string& name) : ParseNode(context, token), m_name(name) {}
		
        void write(AssemblyFileWriter& writer);

        std::string name(){
            return m_name;
        }
};

class FunctionCall : public ParseNode {
    private:
        std::string m_function;

    public:
        FunctionCall(Context* context, const Token* token, const std::string& function) : ParseNode(context, token), m_function(function) {}

        void write(AssemblyFileWriter& writer);
        void checkFunctions(Program& program);
};

} //end of eddic

#endif
