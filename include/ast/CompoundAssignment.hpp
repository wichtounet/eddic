//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_COMPOUND_ASSIGNMENT_H
#define AST_COMPOUND_ASSIGNMENT_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"
#include "ast/Operator.hpp"

namespace eddic {

class Context;

namespace ast {

struct ASTCompoundAssignment {
    std::shared_ptr<Context> context;

    std::string variableName;
    Value value;
    ast::Operator op;
    Position position;

    mutable long references;
    ASTCompoundAssignment() : references(0) {}
};

/*!
 * \typedef Assignment
 * \brief The AST node for an assignment to a variable. 
 */
typedef Deferred<ASTCompoundAssignment> CompoundAssignment;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::CompoundAssignment, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->variableName)
    (eddic::ast::Operator, Content->op)
    (eddic::ast::Value, Content->value)
)

#endif
