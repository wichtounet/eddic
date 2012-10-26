//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_ESCAPE_ANALYSIS_H
#define MTAC_ESCAPE_ANALYSIS_H

#include <unordered_set>
#include <memory>

namespace eddic {

class Variable;

namespace mtac {

class Function;

typedef std::shared_ptr<std::unordered_set<std::shared_ptr<Variable>>> EscapedVariables;

EscapedVariables escape_analysis(std::shared_ptr<mtac::Function> function);

} //end of eddic

} //end of mtac

#endif
