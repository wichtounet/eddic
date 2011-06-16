//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "CompilerException.h"

const char* CompilerException::what() throw() {
	std::string log = file;
	log += ":";
	log += NumberToString<int>(line);
	log += " ";
	log += message;
	
	return log.c_str();
}
