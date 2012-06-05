//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STRUCT_COMPOUND_ASSIGNMENT_H
#define AST_STRUCT_COMPOUND_ASSIGNMENT_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"
#include "ast/Operator.hpp"

namespace eddic {

class Context;

namespace ast {

struct ASTStructCompoundAssignment {
    std::shared_ptr<Context> context;

    std::string variableName;
    std::vector<std::string> memberNames;
    Value value;
    ast::Operator op;
    Position position;

    mutable long references = 0;
};

/*!
 * \typedef Assignment
 * \brief The AST node for an assignment to a variable. 
 */
typedef Deferred<ASTStructCompoundAssignment> StructCompoundAssignment;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::StructCompoundAssignment, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->variableName)
    (std::vector<std::string>, Content->memberNames)
    (eddic::ast::Operator, Content->op)
    (eddic::ast::Value, Content->value)
)

#endif
