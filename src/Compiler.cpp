//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <commons/ByteCode.h>
#include <commons/Timer.h>

#include "Compiler.h"

using std::string;
using std::cout;
using std::endl;

int Compiler::compile(string file){
	cout << "Compile " << file << endl;

	Timer timer;

	string::size_type ext_pos = file.find_last_of( '.' );

	string output = "main.v";

    	if ( ext_pos != string::npos ){
		output = file;
		output.replace( ext_pos + 1, output.size() - 1, "v" );
	}
	
	int code = 0;
	try {
		writer.open(output);

		lexer.lex(file);

		compile();
	} catch (CompilerException e){
		cout << e.what() << endl;
		code = 1;
	}
	
	lexer.close();
	writer.close();

	if(code != 0){
		remove(output.c_str());
	}
	
	cout << "Compilation took " << timer.elapsed() << "ms" << endl;
	
	return code;
}

void Compiler::compile() throw (CompilerException){
	writer.writeHeader();

	Variables variables;

	while(lexer.next()){
		if(!lexer.isWord()){
			throw CompilerException("An instruction can only start with a call or an assignation", __FILE__,__LINE__);
		}

		string word = lexer.getCurrentToken();

		if(!lexer.next()){
			throw CompilerException("Incomplete instruction", __FILE__,__LINE__);
		}

		if(lexer.isLeftParenth()){ //is a call
			parseCall(word, variables);
		} else if(lexer.isWord()){ //is an assign
			parseAssign(word, variables); //TODO Differentiate between assignement and declaration
		} else {
			throw CompilerException("Not an instruction", __FILE__,__LINE__);
		}
	}

	writer.writeEnd();
}

void Compiler::parseCall(string call, Variables& variables) throw (CompilerException){
	if(call != "Print"){
		throw CompilerException("The call \"" + call + "\" does not exist", __FILE__,__LINE__);
	}

	if(!lexer.next()){
		throw CompilerException("Not enough arguments to the call", __FILE__, __LINE__);
	} 

	if(lexer.isLitteral()){
		string litteral = lexer.getCurrentToken();
		writer.writeOneOperandCall(LDCS/*PUSHS*/, litteral);
	} else if(lexer.isWord()){
		string variable = lexer.getCurrentToken();

		if(!variables.exists(variable)){
			throw CompilerException("The variable \"" + variable + "\" does not exist", __FILE__,__LINE__);
		}

		writer.writeOneOperandCall(SLOAD/*PUSHV*/, variables.index(variable));
	} else {
		throw CompilerException("Can only pass litteral or a variable to a call", __FILE__,__LINE__);
	}


	if(!lexer.next() || !lexer.isRightParenth()){
		throw CompilerException("The call must be closed with a right parenth", __FILE__,__LINE__);
	} 

	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon", __FILE__,__LINE__);
	} 

	writer.writeSimpleCall(PRINTS);
} 

void Compiler::parseAssign(string type, Variables& variables) throw (CompilerException){
	if(type != "int" && type != "string"){
		throw CompilerException("Invalid type", __FILE__,__LINE__);
	}

	string variable = lexer.getCurrentToken();

	if(!lexer.next() || !lexer.isAssign()){
		throw CompilerException("A variable declaration must followed by '='", __FILE__,__LINE__);
	} 

	if(!lexer.next()){
		throw CompilerException("Need something to assign to the variable", __FILE__,__LINE__);
	}
	
	if(lexer.isLitteral()){
		if(type != "string"){
			throw new CompilerException("Cannot assign litteral value to a variable of type" + type, __FILE__,__LINE__);
		}
		
		string litteral = lexer.getCurrentToken();

		//add to string pool
		//LDCS stringpool[litteral]
		//SSTORE variables[variable];
	} else if(lexer.isWord()){
		string variableRight = lexer.getCurrentToken();

		if(!variables.exists(variableRight)){
			throw CompilerException("The variable \"" + variableRight + "\" does not exist", __FILE__,__LINE__);
		}

		//Depending on the type of variableRight
	} else if(lexer.isInteger()){
		if(type != "int"){
			throw new CompilerException("Cannot assign integer value to a variable of type" + type, __FILE__,__LINE__);
		}

		string integer = lexer.getCurrentToken();
		int value = toNumber<int>(integer);

		//LDCI value
		//ISTORE variables[variable]
	}
	
	if(!lexer.next() || !lexer.isStop()){
		throw CompilerException("Every instruction must be closed by a semicolon", __FILE__,__LINE__);
	}
}
