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

class Lexer;
class Program;
class ParseNode;
class ElseIf;
class Else;
class Value;
class Condition;

class Parser {
  	private:
		Lexer& lexer;
		
		ParseNode* parseInstruction() throw (CompilerException);
		ParseNode* parseCall(const std::string& call) throw (CompilerException);
		ParseNode* parseDeclaration(const std::string& type) throw (CompilerException);
		ParseNode* parseAssignment(const std::string& variable) throw (CompilerException);
		ParseNode* parseIf() throw (CompilerException);
		Condition* parseCondition() throw (CompilerException);
		ElseIf* parseElseIf() throw (CompilerException);
		Else* parseElse() throw (CompilerException);
		Value* parseValue() throw (CompilerException);
  	
	public:
		Parser(Lexer& l) : lexer(l) {};
		Program* parse() throw (CompilerException) ;
};

#endif
