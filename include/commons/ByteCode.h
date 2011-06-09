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

void writeOneOperandCall(ofstream* outStream, ByteCode bytecode, string litteral);
void writeSimpleCall(ofstream* outStream, ByteCode bytecode);
void writeEnd(ofstream* stream);

ByteCode readByteCode(ifstream* stream);

template<typename T>
std::ostream& binary_write(std::ostream* stream, const T& value){
	return stream->write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template<typename T>
std::istream & binary_read(std::istream* stream, T& value)
{
    return stream->read(reinterpret_cast<char*>(&value), sizeof(T));
}

std::ostream& binary_write_string(std::ostream* stream, string value);

std::string binary_read_string(std::istream* stream);

#endif
