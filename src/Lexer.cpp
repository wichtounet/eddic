//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cctype>

#include "Lexer.h"

using namespace std;

void Lexer::lex(ifstream* inStream){
	stream = inStream;
}

bool Lexer::next(){
	char current;
	*stream >> current;

	while(isspace(current) && !stream->eof()){
		*stream >> current;
	}

	if(stream->eof()){
		return false;
	}
	
	if(current == '"'){
		currentToken = string(1, current);

		while(*stream >> current && current != '"'){
			currentToken += current;
		}

		currentToken += current;
	} else if(isalpha(current)){	
		currentToken = string(1, current);

		while(*stream >> current && isalpha(current)){
			currentToken += current;
		}

		stream->putback(current);
	} else {
		currentToken = string(1, current);
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

bool Lexer::isAssign() const{
	return currentToken == "=";
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
