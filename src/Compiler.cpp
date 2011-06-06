#include <iostream>
#include <iomanip>
#include <fstream>

#include <time.h>

#include "Compiler.h"
#include "ByteCode.h"
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
			cout << "A call must be followed by a left parenth";
			return 1;
		} 
		
		lexer.next();

		if(!lexer.isLitteral()){
			cout << "Can only pass litteral to a call";
			return 1;
		}
		
		string litteral = lexer.getCurrentToken();

		lexer.next();

		if(!lexer.isRightParenth()){
			cout << "The call must be closed with a right parenth";
			return 1;
		}
		
		lexer.next();

		if(!lexer.isStop()){
			cout << "Every instruction must be closed by ;";
			return 1;
		}

		writeOneOperandCall(outStream, PUSH, litteral);
		writeSimpleCall(outStream, PRINT);
		
		lexer.next();
	}
	
	if(lexer.hasMoreToken()){
		cout << "An instruction can only start with a call";
		return 1;
	}

	return 0;
}


void Compiler::writeOneOperandCall(ofstream* outStream, int bytecode, string litteral){
	OneOperandCall call(bytecode, litteral);

	outStream->write((char*)&call, sizeof(call));
}

void Compiler::writeSimpleCall(ofstream* outStream, int bytecode){
	SimpleCall call(bytecode);

	outStream->write((char*)&call, sizeof(call));
}
