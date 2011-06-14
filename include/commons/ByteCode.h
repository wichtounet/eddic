//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BYTECODE_H
#define BYTECODE_H

enum ByteCode {
	PUSHS = 0,
	PRINT = 1,
	END = 2,
	PUSHV = 3,
	ASSIGN = 4
};

#endif
