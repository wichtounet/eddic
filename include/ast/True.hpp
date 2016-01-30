//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_TRUE_H
#define AST_TRUE_H

#include <ostream>

namespace eddic {

namespace ast {

/*!
 * \class True
 * \brief Reprensent a true boolean literal. 
 */
struct True {
    
};

std::ostream& operator<< (std::ostream& stream, True true_);

} //end of ast

} //end of eddic

#endif
