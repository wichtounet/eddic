//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_POSITION_H
#define AST_POSITION_H

#include <string>
#include <ostream>

#include <boost/range/iterator_range.hpp>

namespace eddic {

namespace ast {

/*!
 * \struct Position
 * \brief Define a position in the source code. 
 */
struct Position {
    int file = 0;               /*!< The source file */
    int line = 0;               /*!< The source line number */
    int column = 0;             /*!< The source column number */
};

std::ostream& operator<< (std::ostream& stream, Position position);

bool operator==(const ast::Position& a, const ast::Position& b);
bool operator!=(const ast::Position& a, const ast::Position& b);

} //end of ast

} //end of eddic


#endif
