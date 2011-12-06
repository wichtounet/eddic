//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_RETURN_H
#define TAC_RETURN_H

#include "tac/Argument.hpp"

namespace eddic {

namespace tac {

struct Return {
    Argument arg1;
    Argument arg2;

    Return();
    Return(Argument arg1);
    Return(Argument arg1, Argument arg2);
};

} //end of tac

} //end of eddic

#endif
