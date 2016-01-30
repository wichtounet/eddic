//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_BASIC_BLOCK_EXTRACTOR_H
#define MTAC_BASIC_BLOCK_EXTRACTOR_H

#include <memory>

#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct BasicBlockExtractor {
    void extract(mtac::Program& program) const ;
};

} //end of ast

} //end of eddic

#endif
