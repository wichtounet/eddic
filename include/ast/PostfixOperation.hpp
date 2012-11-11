//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_POSTFIX_OPERATION_H
#define AST_POSTFIX_OPERATION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Operator.hpp"
#include "ast/Position.hpp"
#include "ast/Value.hpp"

namespace eddic {

namespace ast {

struct ASTPostfixOperation {
    Position position;
    Value left_value;
    ast::Operator op;

    mutable long references = 0;
};

typedef Deferred<ASTPostfixOperation> PostfixOperation;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::PostfixOperation, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Value, Content->left_value)
    (eddic::ast::Operator, Content->op)
)

#endif
