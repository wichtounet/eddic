//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BYTECODE_FILE_READER_H
#define BYTECODE_FILE_READER_H

#include <fstream>

#include "commons/ByteCode.hpp"

#include "CompilerException.hpp"

class ByteCodeFileWriter {
  private:
	std::ofstream stream;
  public:
	ByteCodeFileWriter() {};
	void open(const std::string& path) throw (CompilerException) ;
	void close();
	void writeOneOperandCall(ByteCode bytecode, const std::string& litteral);
	void writeOneOperandCall(ByteCode bytecode, int value);
	void writeSimpleCall(ByteCode bytecode);
	void writeHeader();
	void writeEnd();
	void writeLitteral(const std::string& value);
	void writeInt(int value);

	void nativeWrite(std::string);
};

#endif
