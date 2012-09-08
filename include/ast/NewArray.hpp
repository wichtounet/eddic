//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_NEW_ARRAY_H
#define AST_NEW_ARRAY_H

#include <memory>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTNewArray
 * \brief The AST node for a dynamic allocation of array.  
 * Should only be used from the Deferred version (eddic::ast::NewArray).
 */
struct ASTNewArray {
    std::shared_ptr<Context> context;

    Position position;
    Type type;
    Value size;

    mutable long references = 0;
};

/*!
 * \typedef NewArray
 * \brief The AST node for a dynamic allocation of array.
*/
typedef Deferred<ASTNewArray> NewArray;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::NewArray, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->type)
    (eddic::ast::Value, Content->size)
)

#endif
