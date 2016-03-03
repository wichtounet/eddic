//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_CAST_H
#define AST_CAST_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/VariableType.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;
class Type;

namespace ast {

/*!
 * \class Cast
 * \brief The AST node for a cast of a variable to another type.
 */
struct Cast : x3::position_tagged {
    std::shared_ptr<Context> context;
    std::shared_ptr<const eddic::Type> resolved_type;

    Position position;
    Type type;
    Value value;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Cast,
    (eddic::ast::Type, type)
    (eddic::ast::Value, value)
)

#endif
