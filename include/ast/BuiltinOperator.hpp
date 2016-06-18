//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_BUILTIN_OPERATOR_H
#define AST_BUILTIN_OPERATOR_H

#include <memory>
#include <ostream>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Position.hpp"
#include "ast/Value.hpp"

namespace eddic {

namespace ast {

enum class BuiltinType : unsigned int {
    SIZE,
    LENGTH
};

std::ostream& operator<< (std::ostream& stream, BuiltinType type);

/*!
 * \brief The AST node for a function call.
 */
struct BuiltinOperator {
    Position position;
    BuiltinType type;
    std::vector<Value> values;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::BuiltinOperator,
    (eddic::ast::Position, position)
    (eddic::ast::BuiltinType, type)
    (std::vector<eddic::ast::Value>, values)
)

#endif
