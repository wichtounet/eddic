//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef OPERAND_H
#define OPERAND_H

#include <string>

namespace eddic {

class Operand {
    public:
        virtual std::string getValue() = 0;
        virtual bool isImmediate() = 0;
        virtual bool isRegister() = 0;
        virtual bool isStack() = 0;
        virtual bool isGlobal() = 0;
};

} //end of eddic

#endif
