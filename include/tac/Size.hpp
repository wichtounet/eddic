//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
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
