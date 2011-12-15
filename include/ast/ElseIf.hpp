//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_ELSE_IF_H
#define AST_ELSE_IF_H

#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Condition.hpp"

namespace eddic {

namespace ast {

struct ElseIf {
    Condition condition;
    std::vector<Instruction> instructions;
};

} //end of eddic

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ElseIf, 
    (eddic::ast::Condition, condition)
    (std::vector<eddic::ast::Instruction>, instructions)
)

#endif