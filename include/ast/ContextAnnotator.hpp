//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef CONTEXT_ANNOTATOR_H
#define CONTEXT_ANNOTATOR_H

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

/*!
 * \class ContextAnnotator
 * \brief Generate all the symbol tables and reference them in the AST.   
 */
struct ContextAnnotator {
    void annotate(ast::SourceFile& program) const;
};

} //end of ast

} //end of eddic

#endif
