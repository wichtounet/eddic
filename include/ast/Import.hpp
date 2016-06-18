//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_IMPORT_H
#define AST_IMPORT_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class Import
 * \brief The AST node for an import.
 */
struct Import : x3::position_tagged {
    Position position;
    std::string file;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Import,
    (std::string, file)
)

#endif
