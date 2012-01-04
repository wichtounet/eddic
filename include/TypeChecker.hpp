//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "ast/source_def.hpp"

namespace eddic {

/*!
 * \class TypeChecker
 * \brief A checker for type safety in the program. 
 */
struct TypeChecker {
    void check(ast::SourceFile& program) const ;
};

} //end of eddic

#endif
