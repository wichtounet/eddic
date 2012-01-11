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
#include "ast/Value.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ASTFor
 * \brief The AST node for a for loop. 
 * Should only be used from the Deferred version (eddic::ast::For).
 */
struct ASTFor {
    boost::optional<Instruction> start;
    boost::optional<Value> condition;
    boost::optional<Instruction> repeat;
    std::vector<Instruction> instructions;

    mutable long references;
    ASTFor() : references(0) {}
};

/*!
 * \typedef For
 * \brief The AST node for a for loop.
 */
typedef Deferred<ASTFor> For;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::For, 
    (boost::optional<eddic::ast::Instruction>, Content->start)
    (boost::optional<eddic::ast::Value>, Content->condition)
    (boost::optional<eddic::ast::Instruction>, Content->repeat)
    (std::vector<eddic::ast::Instruction>, Content->instructions)
)

#endif
