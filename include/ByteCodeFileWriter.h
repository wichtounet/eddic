//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BYTECODE_FILE_READER_H
#define BYTECODE_FILE_READER_H

#include <iostream>
#include <iomanip>
#include <fstream>

#include "commons/IO.h"
#include "commons/ByteCode.h"

using namespace std;

class ByteCodeFileWriter {
  private:
	ofstream stream;
  public:
	ByteCodeFileWriter(string path);
	void close();
	void writeOneOperandCall(ByteCode bytecode, string litteral);
	void writeOneOperandCall(ByteCode bytecode, int value);
	void writeSimpleCall(ByteCode bytecode);
	void writeHeader();
	void writeEnd();
	void writeLitteral(string value);

};

#endif
