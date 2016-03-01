//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_NEW_ARRAY_H
#define AST_NEW_ARRAY_H

#include <memory>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTNewArray
 * \brief The AST node for a dynamic allocation of array.
 * Should only be used from the Deferred version (eddic::ast::NewArray).
 */
struct NewArray {
    std::shared_ptr<Context> context;

    Position position;
    Type type;
    Value size;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::NewArray,
    (eddic::ast::Position, position)
    (eddic::ast::Type, type)
    (eddic::ast::Value, size)
)

#endif
