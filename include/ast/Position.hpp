//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_POSITION_H
#define AST_POSITION_H

#include <string>

namespace eddic {

namespace ast {

struct Position {
    std::string file;
    std::string theLine;
    int line;
    int column;
};

std::ostream& operator<< (std::ostream& stream, Position position);

} //end of ast

} //end of eddic


#endif
