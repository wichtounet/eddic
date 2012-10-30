//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_PREFIX_OPERATION_H
#define AST_PREFIX_OPERATION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Operator.hpp"
#include "ast/Position.hpp"
#include "ast/LValue.hpp"

namespace eddic {

namespace ast {

struct ASTPrefixOperation {
    Position position;
    LValue left_value;
    ast::Operator op;

    mutable long references = 0;
};

typedef Deferred<ASTPrefixOperation> PrefixOperation;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::PrefixOperation, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Operator, Content->op)
    (eddic::ast::LValue, Content->left_value)
)

#endif
