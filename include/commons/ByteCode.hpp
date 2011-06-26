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

	SLOAD = 10, 
	ILOAD = 11,
	
	SSTORE = 20,
	ISTORE = 21,

	LDCI = 30,
	LDCS = 31,

	PRINTI = 40,
	PRINTS = 41, 

	IADD = 50, 
	SADD = 51,

	ISUB = 60, 

	IMUL = 70, 

	IDIV = 80,
	
	IMOD = 90
};

#endif
