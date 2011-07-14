//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BYTECODE_FILE_READER_H
#define BYTECODE_FILE_READER_H

#include <fstream>

#include "CompilerException.hpp"

class ByteCodeFileWriter {
  private:
	std::ofstream m_stream;
  public:
	ByteCodeFileWriter() {};
	void open(const std::string& path) throw (CompilerException) ;
	void close();
	void writeHeader();
	void writeEnd();

	std::ofstream& stream(){return m_stream;}

	void nativeWrite(std::string);
};

#endif
