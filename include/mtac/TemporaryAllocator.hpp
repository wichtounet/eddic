//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_TEMPORARY_ALLOCATOR_H
#define TAC_TEMPORARY_ALLOCATOR_H

namespace eddic {

namespace mtac {

class Program;

struct TemporaryAllocator {
    void allocate(mtac::Program& program) const ;
};

} //end of ast

} //end of eddic

#endif
