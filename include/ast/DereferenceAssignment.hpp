//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_DEREFERENCE_ASSIGNMENT_H
#define AST_DEREFERENCE_ASSIGNMENT_H

#include <memory>
#include <string>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTDereferenceAssignment
 * \brief The AST node for an assignement to a variable.  
 * Should only be used from the Deferred version (eddic::ast::Assignment).
 */
struct ASTDereferenceAssignment {
    std::shared_ptr<Context> context;

    Position position;
    char op;
    std::string variableName;
    std::vector<std::string> memberNames;
    Value value;

    mutable long references = 0;
};

/*!
 * \typedef Assignment
 * \brief The AST node for an assignment to a variable. 
 */
typedef Deferred<ASTDereferenceAssignment> DereferenceAssignment;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::DereferenceAssignment, 
    (eddic::ast::Position, Content->position)
    (char, Content->op)
    (std::string, Content->variableName)
    (std::vector<std::string>, Content->memberNames)
    (eddic::ast::Value, Content->value)
)

#endif
