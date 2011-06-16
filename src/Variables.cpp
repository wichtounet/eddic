//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variables.h"

using std::string;

Variables::Variables(){
	currentVariable = 0;
}

bool Variables::exists(string variable) const{
	return variables.find(variable) != variables.end();
}

unsigned int Variables::index(std::string variable) const{
	return variables[variable];
}

void Variables::createIfNotExists(string variable){
	if(!exists(variable)){
		variables[variable] = currentVariable++;
	}
}
