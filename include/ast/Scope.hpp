//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_SCOPE_H
#define AST_SCOPE_H

#include <vector>
#include <memory>

namespace eddic {

namespace ast {

/*!
 * \class If
 * \brief The AST node for a if.
 */
struct Scope {
    std::vector<Instruction> instructions;
};

} //end of ast

} //end of eddic

#endif
