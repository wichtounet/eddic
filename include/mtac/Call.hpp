//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_CALL_H
#define MTAC_CALL_H

#include <memory>
#include <string>

namespace eddic {

class Variable;
class Function;

namespace mtac {

struct Call {
    std::string function;
    std::shared_ptr<eddic::Function> functionDefinition;
    std::shared_ptr<Variable> return_;
    std::shared_ptr<Variable> return2_;
    unsigned int depth;

    Call(const std::string& function, std::shared_ptr<eddic::Function> functionDefinition);
    Call(const std::string& function, std::shared_ptr<eddic::Function> functionDefinition, std::shared_ptr<Variable> return_);
    Call(const std::string& function, std::shared_ptr<eddic::Function> functionDefinition, std::shared_ptr<Variable> return_, std::shared_ptr<Variable> return2_);
};

} //end of mtac

} //end of eddic

#endif
