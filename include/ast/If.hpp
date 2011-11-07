//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_IF_H
#define AST_IF_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Condition.hpp"
#include "ast/ElseIf.hpp"
#include "ast/Else.hpp"

namespace eddic {

struct ASTIf {
    ASTCondition condition;
    std::vector<ASTInstruction> instructions;
    std::vector<ASTElseIf> elseIfs;
    boost::optional<ASTElse> else_;
};

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ASTIf, 
    (eddic::ASTCondition, condition)
    (std::vector<eddic::ASTInstruction>, instructions)
    (std::vector<eddic::ASTElseIf>, elseIfs)
    (boost::optional<eddic::ASTElse>, else_)
)

#endif
