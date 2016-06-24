//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_VARIABLE_DECLARATION_H
#define AST_VARIABLE_DECLARATION_H

#include <memory>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Value.hpp"
#include "ast/Position.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class VariableDeclaration
 * \brief The AST node for a declaration of a local variable.
 */
struct VariableDeclaration : x3::file_position_tagged {
    std::shared_ptr<Context> context;

    Position position;
    Type variableType;
    std::string variableName;
    boost::optional<Value> value;

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::VariableDeclaration,
    (eddic::ast::Type, variableType)
    (std::string, variableName)
    (boost::optional<eddic::ast::Value>, value)
)

#endif
