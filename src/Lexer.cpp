#include <iostream>
#include <iomanip>
#include <fstream>

#include "Lexer.h"

using namespace std;

Lexer::Lexer(ifstream* inStream) : stream(inStream) {}

bool Lexer::next(){
	char current;
	*stream >> current;

	if(current == '(' || current == ')' || current == ';'){
		currentToken = string(1, current);
	} else if(current == '"'){
		currentToken = string(1, current);

		while(*stream >> current && current != '"'){
			currentToken += current;
		}

		currentToken += current;
	} else {	
		currentToken = string(1, current);

		while(*stream >> current && !(current == '(' || current == ')' || current == ';')){
			currentToken += current;
		}

		stream->putback(current);
	}

	if(stream->eof()){
		return false;
	}

	if(currentToken[0] == 0){
		return false;
	}
	
	return true;
}

string Lexer::getCurrentToken(){
	return currentToken;
}

bool Lexer::isCall(){
	return !isParenth() && !isLitteral() && !isStop();
}

bool Lexer::isLitteral(){
	return currentToken[0] == '"';
}

bool Lexer::isParenth(){
	return isLeftParenth() || isRightParenth();
}

bool Lexer::isLeftParenth(){
	return currentToken[0] == '(';
}

bool Lexer::isRightParenth(){
	return currentToken[0] == ')';
}

bool Lexer::isStop(){
	return currentToken[0] == ';';
}
