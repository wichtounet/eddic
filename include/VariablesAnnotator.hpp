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

/*!
 * \class VariablesAnnotator
 * \brief Check the variables of the AST
 */
struct VariablesAnnotator {
    /*!
     * \brief Annotate the program
     * 
     * After this phase, all the variables are resolved as entry to the symbol table. 
     */
    void annotate(ast::SourceFile& program) const ;
};

} //end of eddic

#endif
