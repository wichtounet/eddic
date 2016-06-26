//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_STRUCT_DECLARATION_H
#define AST_STRUCT_DECLARATION_H

#include <memory>
#include <vector>

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/Value.hpp"
#include "ast/VariableType.hpp"

namespace eddic {

class Context;

namespace ast {

/*!
 * \class StructDeclaration
 * \brief The AST node for a declaration of a local struct.
 */
struct StructDeclaration : x3::file_position_tagged {
    std::shared_ptr<Context> context;

    Type variableType;
    std::string variableName;
    std::vector<Value> values;

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::StructDeclaration,
    (eddic::ast::Type, variableType)
    (std::string, variableName)
    (std::vector<eddic::ast::Value>, values)
)

#endif
