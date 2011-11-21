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

#include "ast/Deferred.hpp"
#include "ast/Condition.hpp"
#include "ast/ElseIf.hpp"
#include "ast/Else.hpp"

namespace eddic {

namespace ast {

struct ASTIf {
    Condition condition;
    std::vector<Instruction> instructions;
    std::vector<ElseIf> elseIfs;
    boost::optional<Else> else_;
};

typedef Deferred<ASTIf> If;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::If, 
    (eddic::ast::Condition, Content->condition)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
    (std::vector<eddic::ast::ElseIf>, Content->elseIfs)
    (boost::optional<eddic::ast::Else>, Content->else_)
)

#endif
