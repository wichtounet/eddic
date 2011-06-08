#include <iostream>
#include <iomanip>
#include <fstream>

#include <time.h>

#include <commons/ByteCode.h>

#include "Compiler.h"
#include "Lexer.h"

using namespace std;

int Compiler::compile(string file){
	cout << "Compile " << file << endl;

	int start = time(NULL);

	ifstream inFile;
	inFile.open(file.c_str());

	if(!inFile){
		cout << "Unable to open " << file << endl;
		
		return 1;
	}

	ofstream outFile;
	outFile.open("main.v", ios::binary);

	int code = compile(&inFile, &outFile);

	inFile.close();
	outFile.close();

	int end = time(NULL);

	cout << "Compilation finished in " << (end - start) << "ms" << endl;

	return code;
}

int Compiler::compile(ifstream* inStream, ofstream* outStream){
	Lexer lexer(inStream);
	
	lexer.next();

	while(lexer.isCall() && lexer.hasMoreToken()){
		string call = lexer.getCurrentToken();		
		
		lexer.next();
		
		if(!lexer.isLeftParenth()){
			cout << "A call must be followed by a left parenth" << endl;
			return 1;
		} 
		
		if(!lexer.hasMoreToken()){
			cout << "Not enough arguments to the call" << endl;
			return 1;
		} 
		
		lexer.next();

		if(!lexer.isLitteral()){
			cout << "Can only pass litteral to a call" << endl;
			return 1;
		}

		string litteral = lexer.getCurrentToken();
		
		if(!lexer.hasMoreToken()){
			cout << "The call must be closed with a right parenth" << endl;
			return 1;
		} 

		lexer.next();

		if(!lexer.isRightParenth()){
			cout << "The call must be closed with a right parenth" << endl;
			return 1;
		}
		
		if(!lexer.hasMoreToken()){
			cout << "Every instruction must be closed by ;" << endl;
			return 1;
		} 
		
		lexer.next();

		if(!lexer.isStop()){
			cout << "Every instruction must be closed by ;" << endl;
			return 1;
		}

		writeOneOperandCall(outStream, PUSH, litteral);
		writeSimpleCall(outStream, PRINT);
		
		lexer.next();
	}
	
	if(lexer.hasMoreToken()){
		cout << "An instruction can only start with a call" << endl;
		return 1;
	}

	return 0;
}
