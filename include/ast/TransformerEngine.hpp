//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TRANSFORMER_ENGINE_H
#define TRANSFORMER_ENGINE_H

#include "ast/source_def.hpp"

namespace eddic {

namespace ast {

/*!
 * \struct TransformerEngine
 * \brief Transforms the AST.
 *
 * The main transformation is to transform a Foreach in a For loop. Another transformation is the cleanup of composed values
 * to remove some not-composed values introduced by the parsing phase.    
 */
struct TransformerEngine {
    void clean(SourceFile& program) const;
    void transform(SourceFile& program) const;
};

} //end of ast

} //end of eddic

#endif