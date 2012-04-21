//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_CALL_H
#define TAC_CALL_H

#include <memory>

#include "Variable.hpp"

namespace eddic {

class Variable;
class Function;

namespace tac {

struct Call {
    std::string function;
    std::shared_ptr<Function> functionDefinition;
    std::shared_ptr<Variable> return_;
    std::shared_ptr<Variable> return2_;

    Call();
    Call(const std::string& function, std::shared_ptr<Function> functionDefinition);
    Call(const std::string& function, std::shared_ptr<Function> functionDefinition, std::shared_ptr<Variable> return_);
    Call(const std::string& function, std::shared_ptr<Function> functionDefinition, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);
};

} //end of tac

} //end of eddic

#endif
