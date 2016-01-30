//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "cpp_utils/assert.hpp"

#include "mtac/VariableReplace.hpp"
#include "mtac/Quadruple.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

void mtac::VariableReplace::update_usage(mtac::Argument& value){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&value)){
        if(clones.find(*ptr) != clones.end()){
            value = clones[*ptr];
        }
    }
}

void mtac::VariableReplace::update_usage_optional(boost::optional<mtac::Argument>& opt){
    if(opt){
        update_usage(*opt);
    }
}

void mtac::VariableReplace::replace(mtac::Quadruple& quadruple){
    if(clones.find(quadruple.result) != clones.end()){
        cpp_assert(mtac::isVariable(clones[quadruple.result]), "The result cannot be replaced by other thing than a variable");
        quadruple.result = boost::get<std::shared_ptr<Variable>>(clones[quadruple.result]);
    }

    if(quadruple.secondary && clones.find(quadruple.secondary) != clones.end()){
        cpp_assert(mtac::isVariable(clones[quadruple.secondary]), "The return variable cannot be replaced by other thing than a variable");
        quadruple.secondary = boost::get<std::shared_ptr<Variable>>(clones[quadruple.secondary]);
    }

    update_usage_optional(quadruple.arg1);
    update_usage_optional(quadruple.arg2);
}
