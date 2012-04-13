//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_STRUCT_ASSIGNMENT_H
#define AST_STRUCT_ASSIGNMENT_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTStructAssignment
 * \brief The AST node for an assignment to a struct member. 
 * Should only be used from the Deferred version (eddic::ast::StructAssignment).
 */
struct ASTStructAssignment {
    std::shared_ptr<Context> context;

    Position position;
    std::string variableName;
    std::string memberName;
    Value value;

    mutable long references = 0;
};

/*!
 * \typedef StructAssignment
 * \brief The AST node for an assignment to a struct member. 
 */
typedef Deferred<ASTStructAssignment> StructAssignment;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::StructAssignment, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->variableName)
    (std::string, Content->memberName)
    (eddic::ast::Value, Content->value)
)

#endif
