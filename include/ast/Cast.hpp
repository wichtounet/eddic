//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_CAST_H
#define AST_CAST_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Type.hpp"
#include "ast/Position.hpp"

namespace eddic {

namespace ast {

struct ASTCast {
    Value value;
    Type type;
    Position position;

    mutable long references;
    ASTCast() : references(0) {}
};

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
