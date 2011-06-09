#ifndef BYTECODE_H
#define BYTECODE_H

#include <iostream>
#include <iomanip>

using namespace std;

enum ByteCode {
	PUSH = 0,
	PRINT = 1,
	END = 2
};

/* Write operations */

void writeOneOperandCall(ofstream* outStream, ByteCode bytecode, string litteral);
void writeSimpleCall(ofstream* outStream, ByteCode bytecode);
void writeEnd(ofstream* stream);
void writeLitteral(ofstream* stream, string value);

/* Read operations */

ByteCode readByteCode(ifstream* stream);
char readConstantType(ifstream* stream);
string readLitteral(ifstream* stream);

#endif
