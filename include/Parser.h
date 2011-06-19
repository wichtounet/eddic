//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef PARSER_H
#define PARSER_H

#include <fstream>

#include "CompilerException.h"
#include "Lexer.h"
#include "Nodes.h"

class Parser {
  private:
    Lexer& lexer;
  public:
	Parser(Lexer& l) : lexer(l) {};
	Program* parse() throw (CompilerException) ;
};

#endif
