//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cctype>

#include <commons/Types.h>
#include <commons/Utils.h>

#include "Parser.h"

using std::string;
using std::ios_base;

Program* Parser::parse() throw (CompilerException) {
	Program* program = new Program();

	while(lexer.next()){
		if(!lexer.isWord()){
			throw CompilerException("An instruction can only start with a call or an assignation");
		}

		string word = lexer.getCurrentToken();

		if(!lexer.next()){
			throw CompilerException("Incomplete instruction");
		}

		if(lexer.isLeftParenth()){ //is a call
			parseCall(program, word);
		} else if(lexer.isWord()){ //is a declaration
			parseDeclaration(program, word);
		} else if(lexer.isAssign()){ //is an assign
			parseAssignment(program, word);
		} else {
			throw CompilerException("Not an instruction");
		}
	}
	
	return program;
}

ParseNode* readValue(Lexer& lexer){
	if(!lexer.next()){
		throw CompilerException("Waiting for a value");
	} 
	
	ParseNode* node = NULL;

	if(lexer.isLitteral()){		
		string litteral = lexer.getCurrentToken();

		node = new Litteral(litteral);
	} else if(lexer.isWord()){
		string variableRight = lexer.getCurrentToken();
		
		node = new VariableValue(variableRight);
	} else if(lexer.isInteger()){
		string integer = lexer.getCurrentToken();
		int value = toNumber<int>(integer);

		node = new Integer(value);
	}

	lexer.next();

	if(lexer.isAddition()){
		ParseNode* add = new Addition();

		add->addLast(node);
		add->addLast(readValue(lexer));

		return add;
	} else if(lexer.isSubtraction()){
		ParseNode* subtract = new Subtraction();

		subtract->addLast(node);
		subtract->addLast(readValue(lexer));

		return subtract;
	} else {
		lexer.pushBack();
	}

	return node;
}

void Parser::parseCall(Program* program, string call) throw (CompilerException){
	if(call != "Print"){
		throw CompilerException("The call \"" + call + "\" does not exist");
	}

	ParseNode* value = readValue(lexer);
	
	if(!lexer.next() || !lexer.isRightParenth()){
		throw CompilerException("The call must be closed with a right parenth");
	} 

	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon");
	} 

	Print* print = new Print();

	print->addLast(value);

	program->addLast(print);
} 

void Parser::parseDeclaration(Program* program, string typeName) throw (CompilerException){
	if(typeName != "int" && typeName != "string"){
		throw CompilerException("Invalid type");
	}
	
	Type type;
	if(typeName == "int"){
		type = INT;
	} else {
		type = STRING;
	}

	string variable = lexer.getCurrentToken();

	if(!lexer.next() || !lexer.isAssign()){
		throw CompilerException("A variable declaration must followed by '='");
	} 
	
	ParseNode* value = readValue(lexer);
	
	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon");
	}

	Declaration* declare = new Declaration(type, variable);

	declare->addLast(value);

	program->addLast(declare);
}

void Parser::parseAssignment(Program* program, string variable) throw (CompilerException){
	ParseNode* value = readValue(lexer);
	
	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon");
	}

	Assignment* assign = new Assignment(variable); 

	assign->addLast(value);

	program->addLast(assign);
}
