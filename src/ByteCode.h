#ifndef BYTECODE_H
#define BYTECODE_H

#include <iostream>
#include <iomanip>

using namespace std;

static const int PUSH = 0;
static const int PRINT = 1; 

void writeOneOperandCall(ofstream* outStream, int bytecode, string litteral);
void writeSimpleCall(ofstream* outStream, int bytecode);

#endif
