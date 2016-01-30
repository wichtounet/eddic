//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
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
