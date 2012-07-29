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
 * \param function the function signature.
 * \return The mangled function name. 
 */
std::string mangle(const std::shared_ptr<Function> function);
std::string mangle_ctor(const std::shared_ptr<Function> function);
std::string mangle_dtor(const std::shared_ptr<Function> function);

/*!
 * \brief Return the mangled representation of the given function. Used for function calls.  
 * \param functionName The name of the function. 
 * \param values The values used for the call. 
 * \param struct_ The struct of the member function.
 * \return The mangled function name. 
 */
std::string mangle(const std::string& functionName, const std::vector<ast::Value>& values, const std::string& struct_ = "");
std::string mangle_ctor(const std::vector<ast::Value>& values, const std::string& struct_);
std::string mangle_dtor(const std::string& struct_);

/*!
 * \brief Return the mangled representation of the given function. Generic use with a list of types. 
 * \param functionName The name of the function. 
 * \param types The parameters types. 
 * \param struct_ The struct of the member function.
 * \return The mangled function name. 
 */
std::string mangle(const std::string& functionName, const std::vector<std::shared_ptr<const Type>>& types, const std::string& struct_ = "");

} //end of eddic

#endif
