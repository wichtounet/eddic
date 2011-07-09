//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef STRING_POOL_H
#define STRING_POOL_H

#include <string>
#include <map>

#include "ParseNode.hpp"

class StringPool : public ParseNode {
	private:
		std::map<std::string, std::string> pool;
		unsigned int currentString;
	public:
		StringPool();
		std::string label(const std::string& value);
		void write(ByteCodeFileWriter& writer);	
};

#endif
