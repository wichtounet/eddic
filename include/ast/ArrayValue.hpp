//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_ARRAY_VALUE_H
#define AST_ARRAY_VALUE_H

#include <memory>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;
class Variable;

namespace ast {

/*!
 * \class ASTArrayValue
 * \brief The AST node for a value of an array. 
 * Should only be used from the Deferred version (eddic::ast::ArrayValue).
 */
struct ASTArrayValue {
    std::shared_ptr<Context> context;

    Position position;
    std::string arrayName;
    std::shared_ptr<Variable> var;
    Value indexValue;

    mutable long references;
    ASTArrayValue() : references(0) {}
};

/*!
 * \typedef ArrayValue
 * \brief The AST node for a value of an array.
 */
typedef Deferred<ASTArrayValue> ArrayValue;

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ArrayValue, 
    (eddic::ast::Position, Content->position)
    (std::string, Content->arrayName)
    (eddic::ast::Value, Content->indexValue)
)

#endif
