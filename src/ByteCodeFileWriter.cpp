//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "commons/ByteCode.h"
#include "commons/IO.h"

#include "ByteCodeFileWriter.h"

using std::string;
using std::ios;

void ByteCodeFileWriter::open(string path) throw (CompilerException) {
	stream.open(path.c_str(), ios::binary);

	if(!stream){
		throw CompilerException("Unable to open the output file");
	}
}

void ByteCodeFileWriter::close(){
	stream.close();
}

void ByteCodeFileWriter::writeOneOperandCall(ByteCode bytecode, string litteral){
	binary_write(&stream, (int) bytecode);
	
	writeLitteral(litteral);
}

void ByteCodeFileWriter::writeOneOperandCall(ByteCode bytecode, int value){
	binary_write(&stream, (int) bytecode);
	binary_write(&stream, value);
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
	binary_write(&stream, litteral.length() - 2);
	binary_write_string(&stream, litteral.substr(1, litteral.length() - 2));
}

void ByteCodeFileWriter::writeInt(int value){
	binary_write(&stream, value);
}
