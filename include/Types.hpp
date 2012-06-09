//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <memory>

#include "BaseType.hpp"

namespace eddic {

class Type;

extern std::shared_ptr<Type> BOOL;
extern std::shared_ptr<Type> INT;
extern std::shared_ptr<Type> FLOAT;
extern std::shared_ptr<Type> STRING;
extern std::shared_ptr<Type> VOID;

/*!
 * \brief Parse the given type into an EDDI std::shared_ptr<Type>. 
 *
 * \param type The type to parse. 
 */
std::shared_ptr<Type> new_type(const std::string& type);

std::shared_ptr<Type> new_simple_type(const std::string& baseType, bool const_ = false);

std::shared_ptr<Type> new_array_type(BaseType baseType, int size = 0);
std::shared_ptr<Type> new_array_type(const std::string& baseType, int size = 0);

int size(BaseType type);

bool is_standard_type(const std::string& type);

} //end of eddic

#endif
