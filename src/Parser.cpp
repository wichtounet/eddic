//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cctype>

#include <commons/Types.h>

#include "Parser.h"
#include "Value.h"
#include "Call.h"
#include "Assignment.h"
#include "Declaration.h"

using std::string;
using std::ios_base;

void parseCall(Lexer& lexer, Program& program, string call) throw (CompilerException);
void parseDeclaration(Lexer& lexer, Program& program, string type) throw (CompilerException);
void parseAssignment(Lexer& lexer, Program& program, string variable) throw (CompilerException);

Program Parser::parse() throw (CompilerException) {
	Program* program = new Program();

	while(lexer.next()){
		if(!lexer.isWord()){
			throw CompilerException("An instruction can only start with a call or an assignation", __FILE__,__LINE__);
		}

		string word = lexer.getCurrentToken();

		if(!lexer.next()){
			throw CompilerException("Incomplete instruction", __FILE__,__LINE__);
		}

		if(lexer.isLeftParenth()){ //is a call
			parseCall(lexer, *program, word);
		} else if(lexer.isWord()){ //is a declaration
			parseDeclaration(lexer, *program, word);
		} else if(lexer.isAssign()){ //is an assign
			parseAssignment(lexer, *program, word);
		} else {
			throw CompilerException("Not an instruction", __FILE__,__LINE__);
		}
	}
	
	return *program;
}

//TODO Move it to commons (with the sstream include)
template <class T>
T toNumber (std::string text){
	std::stringstream ss(text);
	T result;
	ss >> result;
	return result;
}

Value* readValue(Lexer& lexer){
	if(lexer.isLitteral()){		
		string litteral = lexer.getCurrentToken();

		return new Litteral(litteral);
	} else if(lexer.isWord()){
		string variableRight = lexer.getCurrentToken();
		
		return new Variable(variableRight);
	} else if(lexer.isInteger()){
		string integer = lexer.getCurrentToken();
		int value = toNumber<int>(integer);

		return new Integer(value);
	}

	return NULL;
}

//TODO : put the three following functions as member functions

void parseCall(Lexer& lexer, Program& program, string call) throw (CompilerException){
	if(call != "Print"){
		throw CompilerException("The call \"" + call + "\" does not exist", __FILE__,__LINE__);
	}

	if(!lexer.next()){
		throw CompilerException("Not enough arguments to the call", __FILE__, __LINE__);
	} 
	
	Value* value = readValue(lexer);
	
	if(!lexer.next() || !lexer.isRightParenth()){
		throw CompilerException("The call must be closed with a right parenth", __FILE__,__LINE__);
	} 

	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon", __FILE__,__LINE__);
	} 

	program.addInstruction(new Call(call, value));
} 

void parseDeclaration(Lexer& lexer, Program& program, string typeName) throw (CompilerException){
	if(typeName != "int" && typeName != "string"){
		throw CompilerException("Invalid type", __FILE__,__LINE__);
	}

	//Move that elsewhere (Types.h ?)
	Type type;
	if(typeName == "int"){
		type = INT;
	} else {
		type = STRING;
	}

	string variable = lexer.getCurrentToken();

	if(!lexer.next() || !lexer.isAssign()){
		throw CompilerException("A variable declaration must followed by '='", __FILE__,__LINE__);
	} 

	if(!lexer.next()){
		throw CompilerException("Need something to assign to the variable", __FILE__,__LINE__);
	}
	
	Value* value = readValue(lexer);
	
	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon", __FILE__,__LINE__);
	}

	program.addInstruction(new Declaration(type, variable, value));
}

void parseAssignment(Lexer& lexer, Program& program, string variable) throw (CompilerException){
	if(!lexer.next()){
		throw CompilerException("Need something to assign to the variable", __FILE__,__LINE__);
	}
	
	Value* value = readValue(lexer);
	
	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon", __FILE__,__LINE__);
	}

	program.addInstruction(new Assignment(variable, value));
}
