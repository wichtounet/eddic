//=======================================================================
// Copyright Baptiste Wicht 2011.
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

typedef std::unordered_map<std::shared_ptr<Variable>, std::shared_ptr<Variable>> VariableClones;

struct VariableReplace : public boost::static_visitor<> {
    VariableClones& clones;

    VariableReplace(VariableClones& clones) : clones(clones) {}

    void update_usage(mtac::Argument& value);
    void update_usage_optional(boost::optional<mtac::Argument>& opt);
    
    void operator()(std::shared_ptr<mtac::Quadruple> quadruple);
    void operator()(std::shared_ptr<mtac::Param> param);
    void operator()(std::shared_ptr<mtac::IfFalse> if_false);
    void operator()(std::shared_ptr<mtac::If> if_);
    void operator()(std::shared_ptr<mtac::Call> call_);

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

} //end of mtac

} //end of eddic

#endif
