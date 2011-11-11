//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_BINARY_CONDITION_H
#define AST_BINARY_CONDITION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"

namespace eddic {

namespace ast {

struct ASTBinaryCondition {
    std::string op;
    Value lhs;
    Value rhs;
};

typedef Deferred<ASTBinaryCondition> BinaryCondition;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::BinaryCondition, 
    (eddic::ast::Value, Content->lhs)
    (std::string, Content->op)
    (eddic::ast::Value, Content->rhs)
)

#endif
