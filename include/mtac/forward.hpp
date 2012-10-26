//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_FORWARD_DECLARATIONS_H
#define MTAC_FORWARD_DECLARATIONS_H

#include <memory>
#include <string>

#include "variant.hpp"

namespace eddic {

namespace mtac {

class Program;

class If;
class IfFalse;
class Param;
class Quadruple;
class Goto;
class Call;
class NoOp;

typedef boost::variant<
        std::shared_ptr<mtac::Quadruple>,        //Basic quadruples
        std::shared_ptr<mtac::Param>,            //Parameters
        std::shared_ptr<mtac::IfFalse>,          //Jumping quadruples
        std::shared_ptr<mtac::If>,               //Jumping quadruples
        std::shared_ptr<mtac::Goto>,             //Non-conditional jump
        std::shared_ptr<mtac::Call>,             //Call a function
        std::shared_ptr<mtac::NoOp>,             //Only used by the optimizer
        std::string                              //For labels
    > Statement;

} //end of mtac

} //end of eddic

#endif
