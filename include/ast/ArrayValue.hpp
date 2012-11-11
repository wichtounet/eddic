//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_ARRAY_VALUE_H
#define AST_ARRAY_VALUE_H

#include <memory>
#include <string>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"

namespace eddic {

class Context;
class Variable;

namespace ast {

typedef boost::variant<ast::VariableValue, ast::MemberValue> ArrayRef;

/*!
 * \class ASTArrayValue
 * \brief The AST node for a value of an array. 
 * Should only be used from the Deferred version (eddic::ast::ArrayValue).
 */
struct ASTArrayValue {
    Position position;
    ArrayRef ref;
    Value indexValue;

    mutable long references = 0;
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
    (eddic::ast::ArrayRef, Content->ref)
    (eddic::ast::Value, Content->indexValue)
)

#endif
