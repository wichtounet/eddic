//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_ARRAY_DECLARATION_H
#define AST_ARRAY_DECLARATION_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Position.hpp"
#include "ast/VariableType.hpp"
#include "ast/Value.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class ASTArrayDeclaration
 * \brief The AST node for a declaration of a local array.
 */
struct ArrayDeclaration {
    std::shared_ptr<Context> context;

    Position position;
    Type arrayType;
    std::string arrayName;
    Value size;

    mutable long references = 0;
};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::ArrayDeclaration,
    (eddic::ast::Position, position)
    (eddic::ast::Type, arrayType)
    (std::string, arrayName)
    (eddic::ast::Value, size)
)

#endif
