//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Options.h"
#include "Compiler.h"

#include <iostream>

using std::string;
using std::cout;
using std::endl;

void printUsage();

int main(int argc, const char* argv[]) {
	if(argc == 1){
		cout << "eddic: no input files" << endl;

		return -1;
	}

	for(int i = 1; i < argc; i++){
		string arg = argv[i];

		if(arg[0] != '-'){
			break;
		}

		if(arg == "-o" || arg == "--optimize-all"){
			Options::set(OPTIMIZE_ALL);
		} else if(arg == "--optimize-integers"){
			Options::set(OPTIMIZE_INTEGERS);
		} else if(arg == "--optimize-strings"){
			Options::set(OPTIMIZE_STRINGS);
		} else {
			cout << "Unrecognized option \"" << arg << "\"" << endl;

			printUsage();

			return -1;
		}
	}

	Compiler compiler;
	
	return compiler.compile(argv[argc - 1]);
}

void printUsage(){
	cout << "Usage: eddic [options] file" << endl;

	cout << "Options:" << endl;
	cout << "  -h, --help               Display this information" << endl;
	cout << "  -o, --optimize-all       Enable all optimizations" << endl;
	cout << "  --optimize-strings       Enable the optimizations on strings" << endl;
	cout << "  --optimize-integers      Enable the optimizations on integers" << endl;
}
