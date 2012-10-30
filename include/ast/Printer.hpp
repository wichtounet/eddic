//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast/SourceFile.hpp"

namespace eddic {

namespace ast {

struct Printer {
    void print(SourceFile& program); 
};

} //end of ast

} //end of eddic

#endif
