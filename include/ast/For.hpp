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

#include "ast/Condition.hpp"

namespace eddic {

struct ASTFor {
    boost::optional<ASTInstruction> start;
    boost::optional<ASTCondition> condition;
    boost::optional<ASTInstruction> repeat;
    std::vector<ASTInstruction> instructions;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTFor, 
    (boost::optional<eddic::ASTInstruction>, start)
    (boost::optional<eddic::ASTCondition>, condition)
    (boost::optional<eddic::ASTInstruction>, repeat)
    (std::vector<eddic::ASTInstruction>, instructions)
)

#endif
