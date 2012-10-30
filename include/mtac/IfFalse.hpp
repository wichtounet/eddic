//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_IF_FALSE_H
#define MTAC_IF_FALSE_H

#include <string>
#include <memory>

#include <boost/optional.hpp>

#include "mtac/BinaryOperator.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

namespace mtac {

class BasicBlock;

struct IfFalse {
    Argument arg1;
    boost::optional<BinaryOperator> op;
    boost::optional<Argument> arg2;
    std::string label;
    unsigned int depth;
    
    //Filled only in later phase replacing the label
    std::shared_ptr<BasicBlock> block;

    IfFalse();
    IfFalse(Argument arg1, const std::string& label);
    IfFalse(BinaryOperator op, Argument arg1, Argument arg2, const std::string& label);
};

} //end of mtac

} //end of eddic

#endif
