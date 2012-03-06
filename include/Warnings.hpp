//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef WARNINGS_H
#define WARNINGS_H

#include <string>

#include "ast/Position.hpp"

namespace eddic {

/*!
 * \brief Produces a warning on the command line. 
 * \param warning The warning message to produce.  
 */
void warn(const std::string& warning);

/*!
 * \brief Produces a warning on the command line and display the source of the warning. 
 * \param position The position of the warning in the source file.  
 * \param warning The warning message to produce.  
 */
void warn(const ast::Position& position, const std::string& warning);

} //end of eddic

#endif
