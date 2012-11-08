//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_UNARY_H
#define AST_UNARY_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Operator.hpp"

namespace eddic {

namespace ast {

struct ASTUnary {
    Operator op;
    Value value;

    mutable long references = 0;
};

typedef Deferred<ASTUnary> Unary;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Unary, 
    (eddic::ast::Operator, Content->op)
    (eddic::ast::Value, Content->value)
)

#endif
