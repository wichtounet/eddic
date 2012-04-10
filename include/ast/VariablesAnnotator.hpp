//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef VARIABLES_ANNOTATOR_H
#define VARIABLES_ANNOTATOR_H

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

/*!
 * \brief Annotate the program
 * 
 * After this phase, all the variables are resolved as entry to the symbol table. 
 */
void defineVariables(ast::SourceFile& program);

} //end of ast

} //end of eddic

#endif
