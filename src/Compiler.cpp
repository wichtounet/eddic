//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <commons/ByteCode.h>
#include <commons/Timer.h>

#include "Compiler.h"
#include "Program.h"
#include "Parser.h"

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
		lexer.lex(file);

		Parser parser(lexer);

		Program* program = parser.parse();
		
		//Semantical analysis
			//Type checking
			//Variable checking
			//String pool creation
		
		writer.open(output);

		compile(*program);
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

void compile(Program& program){
	
}
