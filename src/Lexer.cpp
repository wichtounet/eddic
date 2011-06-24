//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cctype>

#include "Lexer.h"

using std::pair;
using std::string;
using std::ios_base;

void Lexer::lex(string file) throw(CompilerException) {
	stream.open(file.c_str());
	stream.unsetf(ios_base::skipws);

	if(!stream){
		throw CompilerException("Unable to open the input file"); 
	}

	currentType = NOTHING;
}

void Lexer::close(){
	stream.close();
}

#include <iostream>

bool Lexer::next(){
	if(!buffer.empty()){
		pair<string, TokenType> old = buffer.top();

		currentToken = old.first;
		currentType = old.second;
		
		buffer.pop();

		return true;
	} else if(readNext()){
		read.push(make_pair(currentToken, currentType));

		return true;
	}

	return false;
}

void Lexer::pushBack(){
	pair<string, TokenType> old = read.top();

	buffer.push(old);

	read.pop();
}

bool Lexer::readNext(){
	char current;
	stream >> current;

	while(isspace(current) && !stream.eof()){
		stream >> current;
	}

	if(stream.eof()){
		currentType = NOTHING;
			
		return false;
	}
	
	if(current == '"'){
		currentToken = string(1, current);

		while(stream >> current && current != '"'){
			currentToken += current;
		}

		currentToken += current;
		
		currentType = LITTERAL;
			
		return true;
	} else if(isalpha(current)){	
		currentToken = string(1, current);

		while(stream >> current && isalpha(current)){
			currentToken += current;
		}

		stream.putback(current);
		
		currentType = WORD;
			
		return true;
	} else if(isdigit(current)) {
		currentToken = string(1, current);

		while(stream >> current && isdigit(current)){
			currentToken += current;
		}

		stream.putback(current);
		
		currentType = INTEGER;
			
		return true;
	} 
	
	switch (current) {
		case ';': 
			currentType = STOP;
			currentToken = ";";
			return true;
		case '=': 
			currentType = ASSIGN;
			currentToken = "=";
			return true;
		case '(': 
			currentType = LEFT_PARENTH;
			currentToken = "(";
			return true;
		case ')': 
			currentType = RIGHT_PARENTH;
			currentToken = ")";
			return true;
		case '+':
			currentType = ADDITION;
			currentToken = "+";
			return true;
		case '-':
			currentType = SUBTRACTION;
			currentToken = "-";
			return true;
		case '*':
			currentType = MULTIPLICATION;
			currentToken = "*";
			return true;
		case '/':
			currentType = DIVISION;
			currentToken = "/";
			return true;
	}
	
	return false;
}

string Lexer::getCurrentToken() const{
	return currentToken;
}

bool Lexer::isWord() const{
	return currentType == WORD;
}

bool Lexer::isLitteral() const{
	return currentType == LITTERAL;
}

bool Lexer::isAssign() const{
	return currentType == ASSIGN;
}

bool Lexer::isParenth() const{
	return currentType == LEFT_PARENTH || currentType == RIGHT_PARENTH;
}

bool Lexer::isLeftParenth() const{
	return currentType == LEFT_PARENTH;
}

bool Lexer::isRightParenth() const{
	return currentType == RIGHT_PARENTH;
}

bool Lexer::isStop() const{
	return currentType == STOP;
}
   
bool Lexer::isInteger() const {
	return currentType == INTEGER;
}

bool Lexer::isAddition() const {
	return currentType == ADDITION;
}

bool Lexer::isSubtraction() const {
	return currentType == SUBTRACTION;
}

bool Lexer::isMultiplication() const {
	return currentType == MULTIPLICATION;
}

bool Lexer::isDivision() const {
	return currentType == DIVISION;
}
