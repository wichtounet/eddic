//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_NEW_H
#define AST_NEW_H

#include <memory>

#include "ast/Deferred.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTNew
 * \brief The AST node for a dynamic allocation.
 * Should only be used from the Deferred version (eddic::ast::New).
 */
struct New {
    std::shared_ptr<Context> context;
    std::string mangled_name;

    Position position;
    Type type;
    std::vector<Value> values;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::New,
    (eddic::ast::Position, position)
    (eddic::ast::Type, type)
    (std::vector<eddic::ast::Value>, values)
)

#endif
