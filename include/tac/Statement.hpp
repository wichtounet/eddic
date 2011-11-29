//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef TAC_STATEMENT_H
#define TAC_STATEMENT_H

#include <boost/variant/variant.hpp>

#include "tac/Quadruple.hpp"
#include "tac/ControlQuadruple.hpp"

namespace eddic {

namespace tac {

boost::variant<Quadruple, ControlQuadruple> Statement;

} //end of tac

} //end of eddic

#endif
