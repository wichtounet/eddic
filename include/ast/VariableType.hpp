//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
typedef boost::variant<SimpleType, ArrayType, TemplateType, PointerType> Type;

bool operator!=(const Type& a, const Type& b);

std::string to_string(const ast::Type& type);

} //end of ast

} //end of eddic

#include "ast/PointerType.hpp"
#include "ast/ArrayType.hpp"
#include "ast/TemplateType.hpp"

#endif
