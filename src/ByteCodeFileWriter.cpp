#include <iostream>
#include <iomanip>
#include <fstream>

#include "commons/ByteCode.h"
#include "commons/IO.h"

#include "ByteCodeFileWriter.h"

using namespace std;

ByteCodeFileWriter::ByteCodeFileWriter(string path){
	stream.open(path.c_str(), ios::binary);
}

void ByteCodeFileWriter::close(){
	stream.close();
}

void ByteCodeFileWriter::writeOneOperandCall(ByteCode bytecode, string litteral){
	binary_write(&stream, (int) bytecode);
	
	writeLitteral(litteral);
}

void ByteCodeFileWriter::writeSimpleCall(ByteCode bytecode){
	binary_write(&stream, (int) bytecode);
}

void ByteCodeFileWriter::writeHeader(){
	binary_write(&stream, (int)('E' + 'D' + 'D' + 'I'));
}

void ByteCodeFileWriter::writeEnd(){
	binary_write(&stream, (int) END);
}

void ByteCodeFileWriter::writeLitteral(string litteral){
	binary_write(&stream, 'S');
	binary_write(&stream, litteral.length() - 2);
	binary_write_string(&stream, litteral.substr(1, litteral.length() - 2));
}
