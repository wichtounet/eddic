//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef AST_TYPE_H
#define AST_TYPE_H

#include "variant.hpp"

#include "ast/SimpleType.hpp"

namespace eddic {

namespace ast {

struct ArrayType;
struct TemplateType;
struct PointerType;

/*!
 * \typedef Type
 * \brief A type in the AST.
 */
typedef boost::spirit::x3::variant<
        SimpleType,
        boost::spirit::x3::forward_ast<ArrayType>,
        boost::spirit::x3::forward_ast<TemplateType>,
        boost::spirit::x3::forward_ast<PointerType>
    > Type;

std::string to_string(const ast::Type& type);

} //end of ast

} //end of eddic

#include "ast/PointerType.hpp"
#include "ast/ArrayType.hpp"
#include "ast/TemplateType.hpp"

X3_FORWARD_AST(eddic::ast::ArrayType)
X3_FORWARD_AST(eddic::ast::TemplateType)
X3_FORWARD_AST(eddic::ast::PointerType)

#endif
