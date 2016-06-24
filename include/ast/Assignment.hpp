//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_ASSIGNMENT_H
#define AST_ASSIGNMENT_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Position.hpp"
#include "ast/Operator.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTAssignment
 * \brief The AST node for an assignement to a variable.
 */
struct Assignment : x3::file_position_tagged {
    std::shared_ptr<Context> context;

    Position position;
    Value left_value;
    Value value;
    ast::Operator op = ast::Operator::ASSIGN; //If not specified, it is not a compound operator

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Assignment,
    (eddic::ast::Value, left_value)
    (eddic::ast::Operator, op)
    (eddic::ast::Value, value)
)

#endif
