//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>

#include <commons/Timer.hpp>

#include "StringPool.hpp"
#include "Variables.hpp"
#include "Compiler.hpp"
#include "Nodes.hpp"
#include "Parser.hpp"

using std::string;
using std::cout;
using std::endl;

int Compiler::compile(string file){
	cout << "Compile " << file << endl;

	Timer timer;

	string::size_type ext_pos = file.find_last_of( '.' );

	string output = "main.asm";

	if ( ext_pos != string::npos ){
		output = file;
		output.replace( ext_pos + 1, output.size() - 1, "asm" );
	}
	
	int code = 0;
	try {
		lexer.lex(file);

		Parser parser(lexer);

		Program* program = parser.parse();

		Variables variables;
		StringPool* pool = new StringPool();

		//Semantical analysis
		check(program, variables);
		checkStrings(program, *pool);
		
		//Optimize the parse tree
		program->optimize();
		writer.open(output);

		compile(program);
		
		delete program;
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

void Compiler::compile(Program* program){
	writer.writeHeader();

	program->write(writer);

	writer.writeEnd();
}

void Compiler::check(Program* program, Variables& variables){
	NodeIterator it = program->begin();
	NodeIterator end = program->end();

	for( ; it != end; ++it){
		ParseNode* node = *it;

		node->checkVariables(variables);
	}
}

void Compiler::checkStrings(Program* program, StringPool& pool){
	NodeIterator it = program->begin();
	NodeIterator end = program->end();

	for( ; it != end; ++it){
		ParseNode* node = *it;

		node->checkStrings(pool);
	}

	program->addFirst(&pool);
}
