//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_OPTIMIZER_H
#define TAC_OPTIMIZER_H

namespace eddic {

class StringPool;

namespace mtac {

class Program;

struct Optimizer {
    void optimize(mtac::Program& program, StringPool& pool) const ;
};

} //end of mtac

} //end of eddic

#endif
