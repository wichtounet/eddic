//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_BASIC_BLOCK_EXTRACTOR_H
#define TAC_BASIC_BLOCK_EXTRACTOR_H

namespace eddic {

namespace tac {

class Program;

struct BasicBlockExtractor {
    void extract(tac::Program& program) const ;
};

} //end of ast

} //end of eddic

#endif