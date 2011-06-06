#include <iostream>
#include <iomanip>
#include <fstream>

#include "ByteCode.h"

using namespace std;

class SimpleCall {
   public:
	int m_bytecode;
	SimpleCall(int bytecode) : m_bytecode(bytecode) {}
};

class OneOperandCall {
   public:
	int m_bytecode;
	std::string m_litteral;
	OneOperandCall(int bytecode, string litteral) : m_bytecode(bytecode), m_litteral(litteral) {}
};

void writeOneOperandCall(ofstream* outStream, int bytecode, string litteral){
	OneOperandCall call(bytecode, litteral);

	outStream->write((char*)&call, sizeof(call));
}

void writeSimpleCall(ofstream* outStream, int bytecode){
	SimpleCall call(bytecode);

	outStream->write((char*)&call, sizeof(call));
}
