//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctype.h>

#include "Lexer.h"

using namespace std;

Lexer::Lexer(ifstream* inStream) : stream(inStream) {}

bool Lexer::next(){
	char current;
	*stream >> current;

	while((current == '\n' || current == ' ') && !stream->eof()){
		*stream >> current;
	}

	if(stream->eof()){
		return false;
	}

	//If is a special char
	if(current == '(' || current == ')' || current == ';' || current == '='){
		currentToken = string(1, current);
	} else if(current == '"'){	//If starts with "
		currentToken = string(1, current);

		while(*stream >> current && current != '"'){
			currentToken += current;
		}

		currentToken += current;
	} else {	
		currentToken = string(1, current);

		while(*stream >> current && isalpha(current)){
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

string Lexer::getCurrentToken() const{
	return currentToken;
}

bool Lexer::isWord() const{
	return isalpha(currentToken[0]);
}

bool Lexer::isLitteral() const{
	return currentToken[0] == '"';
}

bool Lexer::isParenth() const{
	return isLeftParenth() || isRightParenth();
}

bool Lexer::isLeftParenth() const{
	return currentToken == "(";
}

bool Lexer::isRightParenth() const{
	return currentToken == ")";
}

bool Lexer::isStop() const{
	return currentToken == ";";
}
