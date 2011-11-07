//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPTIMIZATION_ENGINE_H
#define OPTIMIZATION_ENGINE_H

namespace eddic {

class ASTProgram;

struct OptimizationEngine {
    void optimize(ASTProgram& program);
};

} //end of eddic

#endif
