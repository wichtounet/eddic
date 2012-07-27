//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_NEW_H
#define AST_NEW_H

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ASTNew
 * \brief The AST node for a dynamic allocation.  
 * Should only be used from the Deferred version (eddic::ast::New).
 */
struct ASTNew {
    Position position;
    Type type;

    mutable long references = 0;
};

/*!
 * \typedef New 
 * \brief The AST node for a dynamic allocation.
*/
typedef Deferred<ASTNew> New;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::New, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->type)
)

#endif
