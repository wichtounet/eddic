//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_FALSE_H
#define AST_FALSE_H

#include <ostream>

namespace eddic {

namespace ast {

/*!
 * \class False
 * \brief Reprensent a false boolean literal. 
 */
struct False {

};

std::ostream& operator<< (std::ostream& stream, False false_);

} //end of ast

} //end of eddic

#endif
