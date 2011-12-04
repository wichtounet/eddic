//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_CALL_H
#define TAC_CALL_H

namespace eddic {

namespace tac {

struct Call {
    std::string function;

    Call();
    Call(const std::string& function);
};

} //end of tac

} //end of eddic

#endif
