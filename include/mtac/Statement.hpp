//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_STATEMENT_H
#define TAC_STATEMENT_H

#include <boost/variant/variant.hpp>

#include "mtac/Quadruple.hpp"
#include "mtac/Param.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/If.hpp"
#include "mtac/Goto.hpp"
#include "mtac/Call.hpp"
#include "mtac/NoOp.hpp"

namespace eddic {

namespace tac {

typedef boost::variant<
        std::shared_ptr<tac::Quadruple>,        //Basic quadruples
        std::shared_ptr<tac::Param>,            //Parameters
        std::shared_ptr<tac::IfFalse>,          //Jumping quadruples
        std::shared_ptr<tac::If>,               //Jumping quadruples
        std::shared_ptr<tac::Goto>,             //Non-conditional jump
        std::shared_ptr<tac::Call>,             //Call a function
        tac::NoOp,                              //Only used by the optimizer
        std::string                             //For labels
    > Statement;

} //end of tac

} //end of eddic

#endif
