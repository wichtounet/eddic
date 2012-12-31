//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STANDARD_IMPORT_H
#define AST_STANDARD_IMPORT_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class StandardImport
 * \brief The AST node for an import of the standard import. 
 */
struct StandardImport {
    Position position;
    std::string header;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::StandardImport, 
    (eddic::ast::Position, position)
    (std::string, header)
)

#endif
