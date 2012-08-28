//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef AST_TYPE_H
#define AST_TYPE_H

#include "variant.hpp"

#include "ast/SimpleType.hpp"
#include "ast/ArrayType.hpp"
#include "ast/PointerType.hpp"
#include "ast/TemplateType.hpp"

namespace eddic {

namespace ast {

/*!
 * \typedef Type
 * \brief A type in the AST.  
 */
typedef boost::variant<SimpleType, ArrayType, TemplateType, PointerType> Type;

std::string to_string(const ast::Type& type);

} //end of ast

} //end of eddic

#endif
