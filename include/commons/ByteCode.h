#ifndef BYTECODE_H
#define BYTECODE_H

#include <iostream>
#include <iomanip>

using namespace std;

enum ByteCode {
	PUSH = 0,
	PRINT = 1
};

void writeOneOperandCall(ofstream* outStream, ByteCode bytecode, string litteral);
void writeSimpleCall(ofstream* outStream, ByteCode bytecode);

#endif
