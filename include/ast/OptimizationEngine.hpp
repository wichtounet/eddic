//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIMIZATION_ENGINE_H
#define OPTIMIZATION_ENGINE_H

#include "ast/source_def.hpp"

namespace eddic {

class FunctionTable;
class StringPool;

namespace ast {

void optimizeAST(ast::SourceFile& program, FunctionTable& functionTable, StringPool& pool);

} //end of ast

} //end of eddic

#endif
