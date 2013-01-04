//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_IF_H
#define MTAC_IF_H

#include <string>
#include <memory>

#include <boost/optional.hpp>

#include "mtac/Operator.hpp"
#include "mtac/Argument.hpp"
#include "mtac/forward.hpp"

namespace eddic {

namespace mtac {

struct If {
    Argument arg1;
    Operator op;
    boost::optional<Argument> arg2;
    std::string label;
    unsigned int depth;
    
    //Filled only in later phase replacing the label
    basic_block_p block;

    If();
    If(Operator op, Argument arg1, const std::string& label);
    If(Operator op, Argument arg1, Argument arg2, const std::string& label);
};

} //end of mtac

} //end of eddic

#endif
