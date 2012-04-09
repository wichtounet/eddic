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

#include "ast/Value.hpp"

#include "Types.hpp"
#include "SymbolTable.hpp"

namespace eddic {

/*!
 * \brief Return the mangled representation of the given type. 
 * \param type The type to mangle. 
 * \return The mangled type. 
 */
std::string mangle(Type type);

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
 * \param symbols The symbol table;
 * \return The mangled function name. 
 */
std::string mangle(const std::string& functionName, const std::vector<ParameterType>& types);

/*!
 * \brief Return the mangled representation of the given function. Used for function calls.  
 * \param functionName The name of the function. 
 * \param values The values used for the call. 
 * \param symbols The symbol table;
 * \return The mangled function name. 
 */
std::string mangle(const std::string& functionName, const std::vector<ast::Value>& values, SymbolTable& symbols);

} //end of eddic

#endif
