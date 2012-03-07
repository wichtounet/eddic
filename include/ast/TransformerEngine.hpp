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
    
void cleanAST(SourceFile& program);
void transformAST(SourceFile& program);

} //end of ast

} //end of eddic

#endif
