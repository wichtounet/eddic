//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_ESCAPE_ANALYSIS_H
#define MTAC_ESCAPE_ANALYSIS_H

#include <unordered_set>
#include <memory>

#include "mtac/forward.hpp"

namespace eddic {

class Variable;

namespace mtac {

typedef std::unordered_set<std::shared_ptr<Variable>> escaped_variables;
typedef std::unique_ptr<std::unordered_set<std::shared_ptr<Variable>>> escaped_variables_ptr;

escaped_variables_ptr escape_analysis(mtac::Function& function);

} //end of eddic

} //end of mtac

#endif
