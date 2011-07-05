//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BYTECODE_H
#define BYTECODE_H

enum ByteCode {
	END = 0, 
	
	LABEL = 5,

	SLOAD = 10, 
	ILOAD = 11,
	
	SSTORE = 20,
	ISTORE = 21,

	LDCI = 30,
	LDCS = 31,
	
	JUMP = 40, 
	JUMP_IF = 41, 
	JUMP_IF_NOT = 42,

	IADD = 50, 
	SADD = 51,

	ISUB = 60, 

	IMUL = 70, 

	IDIV = 80,
	
	IMOD = 90,

	EQUALS = 100, 
	NOT_EQUALS = 101, 
	GREATER_THAN = 102,
	LESS_THAN = 103,
	GREATER_THAN_EQUALS = 104, 
	LESS_THAN_EQUALS = 105,
	
	PRINTI = 110,
	PRINTS = 111, 
};

#endif
