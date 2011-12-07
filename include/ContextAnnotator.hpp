//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONTEXT_ANNOTATOR_H
#define CONTEXT_ANNOTATOR_H

#include "ast/program_def.hpp"

namespace eddic {

/*!
 * \class ContextAnnotator
 * \brief Generate all the symbol tables and reference them in the AST.   
 */
struct ContextAnnotator {
    void annotate(ast::Program& program) const;
};

} //end of eddic

#endif
