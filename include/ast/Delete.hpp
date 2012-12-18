//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DELETE_H
#define AST_DELETE_H

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ASTDelete
 * \brief The AST node for delete a variable. 
 * Should only be used from the Deferred version (eddic::ast::Delete).
 */
struct ASTDelete {
    Position position;
    Value value;

    mutable long references = 0;
};

/*!
 * \typedef Delete
 * \brief The AST node for delete a pointer.
 */
typedef Deferred<ASTDelete> Delete;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Delete, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Value, Content->value)
)

#endif
