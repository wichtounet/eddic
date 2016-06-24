//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_COMPOSED_VALUE_H
#define AST_COMPOSED_VALUE_H

#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/optional.hpp>

#include "ast/Operator.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;
class Function;
class Type;

namespace ast {

typedef boost::tuple<Operator, Value> Operation;
typedef std::vector<Operation> Operations;

/*!
 * \class ASTExpression
 * \brief The AST node for an expression.
 * An expression can be a mathematical expression or a postfix expression (member function calls, array values or member values).
 */
struct Expression : x3::file_position_tagged {
    std::shared_ptr<Context> context;

    Position position;
    Value first;
    Operations operations;
};

bool has_operation_value(const ast::Operation& operation);

} //end of ast

} //end of eddic

//Adapt the structures for the AST

BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Expression,
    (eddic::ast::Value, first)
    (eddic::ast::Operations, operations)
)

#endif
