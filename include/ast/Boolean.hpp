//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_BOOLEAN_H
#define AST_BOOLEAN_H

#include <ostream>

namespace eddic {

namespace ast {

/*!
 * \class Boolean
 * \brief Reprensent a boolean literal.
 */
struct Boolean {
    bool value;
};

std::ostream& operator<< (std::ostream& stream, const Boolean& bool_);

} //end of ast

} //end of eddic

#endif
