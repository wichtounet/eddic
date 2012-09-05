//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_CAST_H
#define AST_CAST_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/VariableType.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTCast
 * \brief The AST node for a cast of a variable to another type.  
 * Should only be used from the Deferred version (eddic::ast::Cast).
 */
struct ASTCast {
    std::shared_ptr<Context> context;

    Position position;
    Type type;
    Value value;

    mutable long references = 0;
};

/*!
 * \typedef Cast
 * \brief The AST node for a cast to another type. 
 */
typedef Deferred<ASTCast> Cast;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Cast, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Type, Content->type)
    (eddic::ast::Value, Content->value)
)

#endif
