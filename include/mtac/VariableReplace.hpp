//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_VARIABLE_REPLACE_H
#define MTAC_VARIABLE_REPLACE_H

#include <unordered_map>
#include <memory>

#include <boost/optional.hpp>

#include "mtac/forward.hpp"
#include "mtac/Argument.hpp"

namespace eddic {

class Variable;

namespace mtac {

typedef std::unordered_map<std::shared_ptr<Variable>, mtac::Argument> VariableClones;

struct VariableReplace {
    VariableClones& clones;

    VariableReplace(VariableClones& clones) : clones(clones) {}

    void update_usage(mtac::Argument& value);
    void update_usage_optional(boost::optional<mtac::Argument>& opt);
    
    void replace(std::shared_ptr<mtac::Quadruple> quadruple);
};

} //end of mtac

} //end of eddic

#endif
