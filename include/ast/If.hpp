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
#include "ast/ElseIf.hpp"
#include "ast/Else.hpp"
#include "ast/Value.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ASTIf
 * \brief The AST node for a if.    
 * Should only be used from the Deferred version (eddic::ast::If).
 */
struct ASTIf {
    Value condition;
    std::vector<Instruction> instructions;
    std::vector<ElseIf> elseIfs;
    boost::optional<Else> else_;

    mutable long references;
    ASTIf() : references(0) {}
};

/*!
 * \typedef If
 * \brief The AST node for a if. 
 */
typedef Deferred<ASTIf> If;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::If, 
    (eddic::ast::Value, Content->condition)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
    (std::vector<eddic::ast::ElseIf>, Content->elseIfs)
    (boost::optional<eddic::ast::Else>, Content->else_)
)

#endif
