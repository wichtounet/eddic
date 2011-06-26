//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cctype>
#include <list>

#include <commons/Types.hpp>
#include <commons/Utils.hpp>

#include "Parser.hpp"
#include "Operators.hpp"

using std::string;
using std::ios_base;
using std::list;

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

ParseNode* readValue(Lexer& lexer);

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

enum Operator {
	ADD, MUL, SUB, DIV, MOD, ERROR
};

class Part {
	public: 
		virtual bool isResolved() = 0;
		virtual Operator getOperator() { return ERROR; }
		virtual Value* getValue(){ return NULL; }
};

class Resolved : public Part {
	public: 	
		Resolved(Value* v) : value(v) {}
		bool isResolved() { return true; }
		Value* getValue() { return value; }

	private: 
		Value* value;
};

class Unresolved : public Part {
	public: 
		Unresolved(Operator o) : op(o) {}
		bool isResolved() { return false; }
		Operator getOperator() { return op; }

	private: 
		Operator op;
};

int priority(Operator op){
	switch(op){
		case MUL:
		case DIV:
		case MOD:
			return 10;
		case ADD:
		case SUB:
			return 0;
		default:
			return -1; //TOOD should never happen
	}
}

ParseNode* readValue(Lexer& lexer){
	list<Part*> parts;

	while(true){
		if(!lexer.next()){
			throw CompilerException("Waiting for a value");
		} 

		Value* node = NULL;

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
		} else {
			throw CompilerException("Invalid value");
		}

		parts.push_back(new Resolved(node));

		if(!lexer.next()){
			break;
		}

		if(lexer.isAddition()){
			parts.push_back(new Unresolved(ADD));
		} else if(lexer.isSubtraction()){
			parts.push_back(new Unresolved(SUB));
		} else if(lexer.isMultiplication()){
			parts.push_back(new Unresolved(MUL));
		} else if(lexer.isDivision()){
			parts.push_back(new Unresolved(DIV));
		} else if(lexer.isModulo()){
			parts.push_back(new Unresolved(MOD));
		} else {
			lexer.pushBack();
			break;
		}
	}

	while(parts.size() > 1){
		int i = 0;
		int debug = 0;
		int maxPriority = -1;
		list<Part*>::iterator it, end, max;
		for(it = parts.begin(), end = parts.end(); it != end; ++it){
			Part* part = *it;

			if(!part->isResolved()){
				if(priority(part->getOperator()) > maxPriority){
					maxPriority = priority(part->getOperator());
					max = it;
					i = debug;
				}
			}

			debug++;
		}

		list<Part*>::iterator first = max;
		--first;

		Part* left = *first;
		Part* center = *max;

		++max;
		Part* right = *max;

		Value* lhs = left->getValue();
		Operator op = center->getOperator();
		Value* rhs = right->getValue();

		Value* value = NULL;

		if(op == ADD){
			value = new Addition(lhs, rhs);
		} else if(op == SUB){
			value = new Subtraction(lhs, rhs);
		} else if(op == MUL) {
			value = new Multiplication(lhs, rhs);
		} else if(op == DIV){
			value = new Division(lhs, rhs);
		} else if(op == MOD){
			value = new Modulo(lhs, rhs);
		}

		parts.erase(first, ++max);
		parts.insert(max, new Resolved(value));
	}

	Value* value = (*parts.begin())->getValue();

	parts.clear();

	return value;
}

