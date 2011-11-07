//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_WHILE_H
#define AST_WHILE_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/While.hpp"
#include "ast/Condition.hpp"

namespace eddic {

struct While {
    ASTCondition condition;
    std::vector<ASTInstruction> instructions;
};

typedef Deferred<While> ASTWhile;

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTWhile, 
    (eddic::ASTCondition, Content->condition)
    (std::vector<eddic::ASTInstruction>, Content->instructions)
)

#endif
