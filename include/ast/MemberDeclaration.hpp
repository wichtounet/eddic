//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_MEMBER_DECLARATION_H
#define AST_MEMBER_DECLARATION_H

#include <boost/fusion/include/adapt_struct.hpp>

#include "ast/VariableType.hpp"

namespace eddic {

namespace ast {

/*!
 * \class MemberDeclaration
 * \brief The AST node for a declaration of a member variable.
 */
struct MemberDeclaration : x3::file_position_tagged {
    Type type;
    std::string name;

};

} //end of ast

} //end of eddic

//Adapt the struct for the AST
BOOST_FUSION_ADAPT_STRUCT(
    eddic::ast::MemberDeclaration,
    (eddic::ast::Type, type)
    (std::string, name)
)

#endif
