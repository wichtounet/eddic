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

struct BinaryCondition {
    std::string op;
    ASTValue lhs;
    ASTValue rhs;
};

typedef Deferred<BinaryCondition> ASTBinaryCondition;

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTBinaryCondition, 
    (eddic::ASTValue, Content->lhs)
    (std::string, Content->op)
    (eddic::ASTValue, Content->rhs)
)

#endif
