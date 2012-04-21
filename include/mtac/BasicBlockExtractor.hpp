//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_BASIC_BLOCK_EXTRACTOR_H
#define MTAC_BASIC_BLOCK_EXTRACTOR_H

namespace eddic {

namespace mtac {

class Program;

struct BasicBlockExtractor {
    void extract(mtac::Program& program) const ;
};

} //end of ast

} //end of eddic

#endif
