//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_QUADRUPLE_H
#define MTAC_QUADRUPLE_H

#include <memory>

#include "tac/Quadruple.hpp"

#include "mtac/Operator.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

class Variable;

namespace mtac {

typedef tac::Quadruple<std::shared_ptr<Variable>, mtac::Argument, mtac::Operator> Quadruple;

} //end of mtac

} //end of eddic

#endif
