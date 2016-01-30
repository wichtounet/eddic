//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef TAC_SIZE_H
#define TAC_SIZE_H

namespace eddic {

namespace tac {

enum class Size : char {
    DEFAULT,
    BYTE,
    WORD,
    DOUBLE_WORD,
    QUAD_WORD
};

} //end of tac

} //end of eddic

#endif
