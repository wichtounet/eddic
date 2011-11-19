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

namespace ast {

struct ASTWhile {
    Condition condition;
    std::vector<Instruction> instructions;
};

typedef Deferred<ASTWhile> While;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::While, 
    (eddic::ast::Condition, Content->condition)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
