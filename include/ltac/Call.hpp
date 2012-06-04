//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_CALL_H
#define LTAC_CALL_H

#include <string>
#include <memory>

namespace eddic {

class Variable;
class Function;

namespace ltac {

struct Call {
    std::string function;
    std::shared_ptr<Function> functionDefinition;

    Call();
    Call(const std::string& function, std::shared_ptr<Function> functionDefinition);
};

} //end of ltac

} //end of eddic

#endif
