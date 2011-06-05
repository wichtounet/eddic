#include <iostream>
#include <iomanip>

#include "Compiler.h"

using namespace std;

int main(int argc, const char* argv[]) {
	if(argc == 1){
		cout << "Not enough arguments. Provide a file to compile" << endl;
	}

	Compiler compiler;
	
	return compiler.compile(argv[1]);
}
