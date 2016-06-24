//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_TERNARY_H
#define AST_TERNARY_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Position.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ASTTernary
 * \brief The AST node for a ternary operator.
 */
struct Ternary : x3::file_position_tagged {
    Position position;
    Value condition;
    Value true_value;
    Value false_value;

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Ternary,
    (eddic::ast::Value, condition)
    (eddic::ast::Value, true_value)
    (eddic::ast::Value, false_value)
)

#endif
