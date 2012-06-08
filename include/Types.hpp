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

extern Type BOOL;
extern Type INT;
extern Type FLOAT;
extern Type STRING;
extern Type VOID;

/*!
 * \brief Parse the given type into an EDDI Type. 
 *
 * \param type The type to parse. 
 */
Type new_type(const std::string& type);

Type new_simple_type(const std::string& baseType, bool const_ = false);

Type new_array_type(BaseType baseType, int size = 0);
Type new_array_type(const std::string& baseType, int size = 0);

int size(BaseType type);
int size(Type type);

bool is_standard_type(const std::string& type);

bool operator==(const Type& lhs, const Type& rhs);
bool operator!=(const Type& lhs, const Type& rhs);

bool operator==(const Type& lhs, const BaseType& rhs);
bool operator!=(const Type& lhs, const BaseType& rhs);

} //end of eddic

#endif
