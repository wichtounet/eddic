//=======================================================================
// Copyright Baptiste Wicht 2011.
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

#include "ast/Deferred.hpp"
#include "ast/Operator.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;

namespace ast {

typedef boost::tuple<Operator, Value> Operation;
typedef std::vector<Operation> Operations;

/*!
 * \class ASTExpression
 * \brief The AST node for a composed value.   
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

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::Expression, 
    (eddic::ast::Position, Content->position)
    (eddic::ast::Value, Content->first)
    (eddic::ast::Operations, Content->operations)
)

#endif
