//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MANGLING_H
#define MANGLING_H

#include <vector>
#include <string>
#include <memory>

#include "ast/Value.hpp"

namespace eddic {

class Type;

/*!
 * \brief Return the mangled representation of the given type. 
 * \param type The type to mangle. 
 * \return The mangled type. 
 */
std::string mangle(std::shared_ptr<const Type> type);

/*!
 * \brief Return the signature of the function from the mangled representation. 
 * \param mangled The mangled representation of the function. 
 * \return The function signature. 
 */
std::string unmangle(std::string mangled);

/*!
 * \brief Return the mangled representation of the given function. Used for function declarations.  
 * \param functionName The name of the function
 * \param types The types of parameters. 
 * \return The mangled function name. 
 */
std::string mangle(const std::string& functionName, const std::vector<ParameterType>& types);

/*!
 * \brief Return the mangled representation of the given function. Used for function calls.  
 * \param functionName The name of the function. 
 * \param values The values used for the call. 
 * \return The mangled function name. 
 */
std::string mangle(const std::string& functionName, const std::vector<ast::Value>& values);

/*!
 * \brief Return the mangled representation of the given function. Generic use with a list of types. 
 * \param functionName The name of the function. 
 * \param types The parameters types. 
 * \return The mangled function name. 
 */
std::string mangle(const std::string& functionName, const std::vector<std::shared_ptr<const Type>>& types);

} //end of eddic

#endif
