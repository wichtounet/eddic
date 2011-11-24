//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIMIZATION_ENGINE_H
#define OPTIMIZATION_ENGINE_H

#include "ast/program_def.hpp"

namespace eddic {

class FunctionTable;
class StringPool;

struct OptimizationEngine {
    void optimize(ast::Program& program, FunctionTable& functionTable, StringPool& pool);
};

} //end of eddic

#endif
