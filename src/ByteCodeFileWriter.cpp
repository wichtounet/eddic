//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "commons/ByteCode.hpp"
#include "commons/IO.hpp"

#include "ByteCodeFileWriter.hpp"

using std::string;
using std::ios;

void ByteCodeFileWriter::open(const std::string& path) throw (CompilerException) {
	m_stream.open(path.c_str());

	if(!m_stream){
		throw CompilerException("Unable to open the output file");
	}
}

void ByteCodeFileWriter::close(){
	m_stream.close();
}

void ByteCodeFileWriter::writeOneOperandCall(ByteCode bytecode, const std::string& litteral){
	//binary_write(&stream, (int) bytecode);
	
	//writeLitteral(litteral);
}

void ByteCodeFileWriter::writeOneOperandCall(ByteCode bytecode, int value){
	//binary_write(&stream, (int) bytecode);
	//binary_write(&stream, value);
}

void ByteCodeFileWriter::writeSimpleCall(ByteCode bytecode){
	//binary_write(&stream, (int) bytecode);
}

void ByteCodeFileWriter::writeHeader(){
        nativeWrite(".text");
        nativeWrite(".globl main");

        nativeWrite("main:");
}

void writePrintString(std::ofstream& m_stream){
	m_stream << "print_string:" << std::endl;
		m_stream << "pushl %ebp" << std::endl;
		m_stream << "movl %esp, %ebp" << std::endl;
		m_stream << "movl $0, %esi" << std::endl;

		m_stream << "movl $4, %eax" << std::endl;
		m_stream << "movl $1, %ebx" << std::endl;
		m_stream << "movl 12(%ebp), %ecx" << std::endl;
		m_stream << "movl 8(%ebp), %edx" << std::endl;
		m_stream << "int $0x80" << std::endl;

		m_stream << "leave" << std::endl;
		m_stream << "ret" << std::endl;
}





void writePrintInteger(std::ofstream& m_stream){
	m_stream << "print_integer:" << std::endl 
		 << "pushl %ebp" << std::endl
		 << "movl %esp, %ebp" << std::endl
		 << "movl 8(%ebp), %eax" << std::endl
		 << "xorl %esi, %esi" << std::endl

	 << "loop:" << std::endl
		 << "movl $0, %edx" << std::endl
		 << "movl $10, %ebx" << std::endl
		 << "divl %ebx" << std::endl
		 << "addl $48, %edx" << std::endl
		 << "pushl %edx" << std::endl
		 << "incl %esi" << std::endl
		 << "cmpl $0, %eax" << std::endl
		 << "jz   next" << std::endl
		 << "jmp loop" << std::endl

	 << "next:" << std::endl
		 << "cmpl $0, %esi" << std::endl
		 << "jz   exit" << std::endl
		 << "decl %esi" << std::endl
		
		 << "movl $4, %eax" << std::endl
		 << "movl %esp, %ecx" << std::endl
		 << "movl $1, %ebx" << std::endl
		 << "movl $1, %edx" << std::endl
		 << "int  $0x80" << std::endl
		
		 << "addl $4, %esp" << std::endl

		 << "jmp  next" << std::endl

	 << "exit:" << std::endl
		 << "movl %ebp, %esp " << std::endl
		 << "popl (%ebp)" << std::endl
		 << "ret" << std::endl;
}

void ByteCodeFileWriter::writeEnd(){
	nativeWrite("mov $1, %eax");
        nativeWrite("mov $0, %ebx");
        nativeWrite("int $0x80");

	//TODO Write only if necessary
	writePrintString(m_stream);
	writePrintInteger(m_stream);
}

void ByteCodeFileWriter::writeLitteral(const std::string& litteral){
	//binary_write(&stream, litteral.length() - 2);
	//binary_write(&stream, litteral.substr(1, litteral.length() - 2));
}

void ByteCodeFileWriter::writeInt(int value){
	//binary_write(&stream, value);
}

void ByteCodeFileWriter::nativeWrite(std::string instruction){
	m_stream << instruction << std::endl;
}
