//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef FUNCTION_H
#define FUNCTION_H

#include <vector>
#include <string>
#include <memory>

#include "ParseNode.hpp"
#include "Types.hpp"

namespace eddic {

class Parameter;

class Function : public ParseNode {
	private:
		std::string m_name;
        std::vector<std::shared_ptr<Parameter>> m_parameters;
        int m_currentPosition;

	public:
		Function(std::shared_ptr<Context> context, const std::shared_ptr<Token> token, const std::string& name) : ParseNode(context, token), m_name(name), m_currentPosition(0) {}
		
        void write(AssemblyFileWriter& writer);

        std::string name();
        std::string mangledName();

        void addParameter(std::string name, Type type);
};

} //end of eddic

#endif
