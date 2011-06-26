//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARSER_H
#define PARSER_H

#include <string>

#include "CompilerException.hpp"
#include "Lexer.hpp"
#include "Nodes.hpp"

class Parser {
  	private:
    	Lexer& lexer;
  	public:
		Parser(Lexer& l) : lexer(l) {};
		Program* parse() throw (CompilerException) ;
		void parseCall(Program* program, std::string call) throw (CompilerException);
		void parseDeclaration(Program* program, std::string type) throw (CompilerException);
		void parseAssignment(Program* program, std::string variable) throw (CompilerException);
};

#endif
