//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>

#include <stdio.h>

#include <commons/ByteCode.h>
#include <commons/Timer.h>

#include "ByteCodeFileWriter.h"
#include "Compiler.h"
#include "Lexer.h"

#include <unistd.h>

using namespace std;

int Compiler::compile(string file){
	cout << "Compile " << file << endl;

	Timer timer;

	ifstream inFile;
	inFile.open(file.c_str());
	inFile.unsetf(ios_base::skipws);

	if(!inFile){
		cout << "Unable to open " << file << endl;
		
		return 1;
	}

	size_t ext_pos = file.find_last_of( '.' );

	string output = "main.v";

    	if ( ext_pos != string::npos ){
		output = file;
		output.replace( ext_pos + 1, output.size() - 1, "v" );
	}

	ByteCodeFileWriter writer(output);
	
	int code = compile(&inFile, &writer);

	inFile.close();
	writer.close();

	if(code != 0){
		remove(output.c_str());
	}
	
	cout << "Compilation took " << timer.elapsed() << "ms" << endl;
	
	return code;
}

int Compiler::compile(ifstream* inStream, ByteCodeFileWriter* writer){
	Lexer lexer(inStream);
	
	writer->writeHeader();

	map<string, int> variables;
	int currentVariable = 0;

	while(lexer.next()){
		if(!lexer.isWord()){
			cout << "An instruction can only start with a call or an assignation" << endl;

			return 1;
		}

		string word = lexer.getCurrentToken();

		if(!lexer.next()){
			cout << "Incomplete instruction" << endl;

			return 1;
		}

		if(lexer.isLeftParenth()){ //is a call
			if(word != "Print"){
				cout << "The call \"" << word << "\" does not exist" << endl;

				return 1;	
			}

			if(!lexer.next()){
				cout << "Not enough arguments to the call" << endl;
				return 1;
			} 
		
			if(lexer.isLitteral()){
				string litteral = lexer.getCurrentToken();
				writer->writeOneOperandCall(PUSHS, litteral);
			} else if(lexer.isWord()){
				string variable = lexer.getCurrentToken();

				if(variables.find(variable) == variables.end()){
					cout << "The variable \"" << variable << "\" does not exist" << endl;
					
					return 1;
				}

				writer->writeOneOperandCall(PUSHV, variable);
			} else {
				cout << "Can only pass litteral or a variable to a call" << endl;
				return 1;
			}

		
			if(!lexer.next() || !lexer.isRightParenth()){
				cout << "The call must be closed with a right parenth" << endl;
				return 1;
			} 
		
			if(!lexer.next() || !lexer.isStop()){
				cout << "Every instruction must be closed by ;" << endl;
				return 1;
			} 
		
			writer->writeSimpleCall( PRINT);
		} else if(lexer.isAssign()){ //is an assign
			if(!lexer.next() || !lexer.isLitteral()){
				cout << "Need a litteral on the right part of the assignation" << endl;
				return 1;
			} 

			string litteral = lexer.getCurrentToken();
		
			if(!lexer.next() || !lexer.isStop()){
				cout << "Every instruction must be closed by ;" << endl;
				return 1;
			}
			
			if(variables.find(word) == variables.end()){
				variables[word] = currentVariable++;
			}
			
			writer->writeOneOperandCall(PUSHS, litteral);
			writer->writeOneOperandCall(ASSIGN, variables[word]);
		} else {
			cout << "Not an instruction " << endl;

			return 1;
		}
	}

	writer->writeEnd();

	return 0;
}
