//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_IF_H
#define TAC_IF_H

#include <string>
#include <memory>
#include <unordered_map>

#include <boost/optional.hpp>

#include "mtac/BinaryOperator.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

class Variable;

namespace mtac {

class BasicBlock;

struct If {
    Argument arg1;
    boost::optional<BinaryOperator> op;
    boost::optional<Argument> arg2;
    std::string label;
    
    std::unordered_map<std::shared_ptr<Variable>, bool> liveness;
    
    //Filled only in later phase replacing the label
    std::shared_ptr<BasicBlock> block;

    If();
    If(Argument arg1, const std::string& label);
    If(BinaryOperator op, Argument arg1, Argument arg2, const std::string& label);
};

} //end of tac

} //end of eddic

#endif
