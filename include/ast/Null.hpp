//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_NULL_H
#define AST_NULL_H

#include <ostream>

namespace eddic {

namespace ast {

/*!
 * \class Null
 * \brief Represent a null pointer. 
 */
struct Null {
    
};

std::ostream& operator<< (std::ostream& stream, Null);

} //end of ast

} //end of eddic

#endif
