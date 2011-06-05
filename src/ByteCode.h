#ifndef BYTECODE_H
#define BYTECODE_H

#include <iostream>
#include <iomanip>

using namespace std;

static const int PUSH = 0;
static const int PRINT = 1; 

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

#endif
