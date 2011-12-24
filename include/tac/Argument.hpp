//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_ARGUMENT_H
#define TAC_ARGUMENT_H

#include <memory>
#include <string>

#include <boost/variant/variant.hpp>

namespace eddic {

class Variable;

namespace tac {

typedef boost::variant<
        std::shared_ptr<Variable>,  //A symbol
        int,                        //An immediate int value
        std::string                 //An immediate string value : A label
    > Argument;

} //end of tac

} //end of eddic

#endif
