//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef DEFAULT_VALUES_H
#define DEFAULT_VALUES_H

#include "ast/program_def.hpp"

namespace eddic {
    
/*!
 * \class DefaultValues
 * \brief Fill the AST with default values for local and global variables.  
 */
struct DefaultValues {
    void fill(ast::Program& program) const ;
};

} //end of eddic

#endif
