#include <iostream>
#include <iomanip>
#include <fstream>

#include <commons/ByteCode.h>
#include <commons/Timer.h>

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

	ofstream outFile;
	outFile.open("main.v", ios::binary);

	int code = compile(&inFile, &outFile);

	inFile.close();
	outFile.close();
	
	cout << "Compilation took " << timer.elapsed() << "ms" << endl;
	
	return code;
}

int Compiler::compile(ifstream* inStream, ofstream* outStream){
	Lexer lexer(inStream);
	
	while(lexer.next()){
		if(!lexer.isCall()){
			cout << "An instruction can only start with a call" << endl;

			return 1;
		}

		string call = lexer.getCurrentToken();

		if(call != "PRINT"){
			cout << "The call \"" << call << "\" does not exist" << endl;

			return 1;	
		}	
		
		if(!lexer.next() || !lexer.isLeftParenth()){
			cout << "A call must be followed by a left parenth" << endl;
			return 1;
		} 
		
		if(!lexer.next()){
			cout << "Not enough arguments to the call" << endl;
			return 1;
		} 
		
		if(!lexer.isLitteral()){
			cout << "Can only pass litteral to a call" << endl;
			return 1;
		}

		string litteral = lexer.getCurrentToken();
		
		if(!lexer.next() || !lexer.isRightParenth()){
			cout << "The call must be closed with a right parenth" << endl;
			return 1;
		} 
		
		if(!lexer.next() || !lexer.isStop()){
			cout << "Every instruction must be closed by ;" << endl;
			return 1;
		} 
		
		writeOneOperandCall(outStream, PUSH, litteral);
		writeSimpleCall(outStream, PRINT);
	}

	writeEnd(outStream);

	return 0;
}
