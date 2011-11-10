//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_FOR_H
#define AST_FOR_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Condition.hpp"

namespace eddic {

struct For {
    boost::optional<ASTInstruction> start;
    boost::optional<ASTCondition> condition;
    boost::optional<ASTInstruction> repeat;
    std::vector<ASTInstruction> instructions;
};

typedef Deferred<For> ASTFor;

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTFor, 
    (boost::optional<eddic::ASTInstruction>, Content->start)
    (boost::optional<eddic::ASTCondition>, Content->condition)
    (boost::optional<eddic::ASTInstruction>, Content->repeat)
    (std::vector<eddic::ASTInstruction>, Content->instructions)
)

#endif
