//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_STATEMENT_H
#define MTAC_STATEMENT_H

#include <boost/variant/variant.hpp>

#include "mtac/Quadruple.hpp"
#include "mtac/Param.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/If.hpp"
#include "mtac/Goto.hpp"
#include "mtac/Call.hpp"
#include "mtac/NoOp.hpp"

namespace eddic {

namespace mtac {

typedef boost::variant<
        std::shared_ptr<mtac::Quadruple>,        //Basic quadruples
        std::shared_ptr<mtac::Param>,            //Parameters
        std::shared_ptr<mtac::IfFalse>,          //Jumping quadruples
        std::shared_ptr<mtac::If>,               //Jumping quadruples
        std::shared_ptr<mtac::Goto>,             //Non-conditional jump
        std::shared_ptr<mtac::Call>,             //Call a function
        mtac::NoOp,                              //Only used by the optimizer
        std::string                             //For labels
    > Statement;

} //end of mtac

} //end of eddic

#endif
