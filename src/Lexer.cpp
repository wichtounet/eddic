//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <cctype>

#include "Lexer.hpp"

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
	if(stream.eof()){
		return false;
	}	

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
	
		if(currentToken == "true"){
			currentType = TRUE;
		} else if(currentToken == "false"){
			currentType = FALSE;
		} else if(currentToken == "if"){
			currentType = IF;
		} else if(currentToken == "else"){
			currentType = ELSE;
		} else {
			currentType = WORD;
		}

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
			stream >> current;

			if(current == '='){
				currentType = EQUALS_TOKEN;
			} else {
				stream.putback(current);
			
				currentType = ASSIGN;
			}

			return true;
		case '(': 
			currentType = LEFT_PARENTH;
			return true;
		case ')': 
			currentType = RIGHT_PARENTH;
			return true;
		case '{': 
			currentType = LEFT_BRACE;
			return true;
		case '}': 
			currentType = RIGHT_BRACE;
			return true;
		case '+':
			currentType = ADDITION;
			return true;
		case '-':
			currentType = SUBTRACTION;
			return true;
		case '*':
			currentType = MULTIPLICATION;
			return true;
		case '/':
			currentType = DIVISION;
			return true;
		case '%':
			currentType = MODULO;
			return true;
		case '!':
			stream >> current;

			if(current == '='){
				currentType = NOT_EQUALS_TOKEN;
			} else {
				return false;
			}

			return true;
		case '<':
			stream >> current;

			if(current == '='){
				currentType = LESS_EQUALS_TOKEN;
			} else {
				stream.putback(current);
				
				currentType = LESS_TOKEN;
			}

			return true;
		case '>':
			stream >> current;			

			if(current == '='){
				currentType = GREATER_EQUALS_TOKEN;
			} else {
				stream.putback(current);
				
				currentType = GREATER_TOKEN;
			}

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

bool Lexer::isLeftBrace() const{
	return currentType == LEFT_BRACE;
}

bool Lexer::isRightBrace() const{
	return currentType == RIGHT_BRACE;
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

bool Lexer::isModulo() const {
	return currentType == MODULO;
}

bool Lexer::isDivision() const {
	return currentType == DIVISION;
}

bool Lexer::isEquals() const {
	return currentType == EQUALS_TOKEN;
}

bool Lexer::isNotEquals() const {
	return currentType == NOT_EQUALS_TOKEN;
}

bool Lexer::isGreater() const {
	return currentType == GREATER_TOKEN;
}

bool Lexer::isLess() const {
	return currentType == LESS_TOKEN;
}

bool Lexer::isGreaterOrEquals() const {
	return currentType == GREATER_EQUALS_TOKEN;
}

bool Lexer::isLessOrEquals() const {
	return currentType == LESS_EQUALS_TOKEN;
}

bool Lexer::isIf() const {
	return currentType == IF;
}

bool Lexer::isElse() const {
	return currentType == ELSE;
}

bool Lexer::isBooleanOperator() const {
	return currentType >= EQUALS_TOKEN && currentType <= LESS_EQUALS_TOKEN;
}

bool Lexer::isBoolean() const {
	return currentType == TRUE || currentType == FALSE;
}

bool Lexer::isTrue() const {
	return currentType == TRUE;
}

bool Lexer::isFalse() const {
	return currentType == FALSE;
}
