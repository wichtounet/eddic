//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

struct Printer {
    void print(SourceFile& program); 
};

} //end of ast

} //end of eddic

#endif
