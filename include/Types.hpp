//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPES_H
#define TYPES_H

#include <string>

#include "Type.hpp"

namespace eddic {

bool operator==(const Type& lhs, const Type& rhs);
bool operator!=(const Type& lhs, const Type& rhs);

bool operator==(const Type& lhs, const BaseType& rhs);
bool operator!=(const Type& lhs, const BaseType& rhs);

/*!
 * \brief Parse the given type into an EDDI Type. 
 *
 * The type must be a standard scalar type or a standard array type. 
 *
 * \param type The type to parse. 
 */
Type newType(const std::string& type);

Type newSimpleType(BaseType baseType, bool const_ = false);
Type newSimpleType(const std::string& baseType, bool const_ = false);

Type newArrayType(BaseType baseType, int size = 0);
Type newArrayType(const std::string& baseType, int size = 0);

Type new_custom_type(const std::string& type);

int size(BaseType type);
int size(Type type);

bool is_standard_type(const std::string& type);

} //end of eddic

#endif
