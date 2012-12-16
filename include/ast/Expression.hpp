//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_COMPOSED_VALUE_H
#define AST_COMPOSED_VALUE_H

#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/optional.hpp>

#include "ast/Deferred.hpp"
#include "ast/Operator.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;
class Function;

namespace ast {

struct CallOperationValue {
    std::string function_name;
    std::vector<ast::Type> template_types;
    std::vector<ast::Value> values;
    std::string mangled_name;
    std::shared_ptr<eddic::Function> function;
    std::shared_ptr<const Type> left_type;
};

typedef boost::variant<
        Value, 
        std::string, 
        CallOperationValue
    >  OperationValueVariant;

typedef boost::optional<OperationValueVariant> OperationValue;
typedef boost::tuple<Operator, OperationValue> Operation;
typedef std::vector<Operation> Operations;

/*!
 * \class ASTExpression
 * \brief The AST node for an expression. 
 * An expression can be a mathematical expression or a postfix expression (member function calls, array values or member values). 
 * Should only be used from the Deferred version (eddic::ast::Expression).
 */
struct ASTExpression {
    std::shared_ptr<Context> context;

    Position position;
    Value first;
    Operations operations;

    mutable long references = 0;
};

/*!
 * \typedef Expression
 * \brief The AST node for a composed value. 
 */
typedef Deferred<ASTExpression> Expression;

} //end of ast

} //end of eddic

//Adapt the structures for the AST

BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Expression, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Value, Content->first)
    (eddic::ast::Operations, Content->operations)
)

BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::CallOperationValue, 
    (std::string, function_name)
    (std::vector<eddic::ast::Type>, template_types)
    (std::vector<eddic::ast::Value>, values)
)

#endif
