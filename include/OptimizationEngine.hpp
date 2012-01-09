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

/*!
 * \class OptimizationEngine
 * \brief This optimizer edit the AST in order to optimize it for runtime performances and for size.  
 */
struct OptimizationEngine {
    void optimize(ast::SourceFile& program, FunctionTable& functionTable, StringPool& pool) const ;
};

} //end of eddic

#endif
