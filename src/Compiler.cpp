#include <iostream>
#include <iomanip>
#include <fstream>

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
	
	cout << "Compilation took " << timer.elapsed() << "ms" << endl;
	
	return code;
}

int Compiler::compile(ifstream* inStream, ByteCodeFileWriter* writer){
	Lexer lexer(inStream);
	
	writer->writeHeader();

	while(lexer.next()){
		if(!lexer.isCall()){
			cout << "An instruction can only start with a call" << endl;

			return 1;
		}

		string call = lexer.getCurrentToken();

		if(call != "Print"){
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
		
		writer->writeOneOperandCall(PUSH, litteral);
		writer->writeSimpleCall( PRINT);
	}

	writer->writeEnd();

	return 0;
}
