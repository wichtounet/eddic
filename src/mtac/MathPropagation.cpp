//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"
#include "Type.hpp"

#include "mtac/MathPropagation.hpp"
#include "mtac/OptimizerUtils.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

void mtac::MathPropagation::clear(){
    optimized = false;
    assigns.clear();
    usage.clear();
}

void mtac::MathPropagation::operator()(mtac::Quadruple& quadruple){
    if(pass == mtac::Pass::DATA_MINING){
        if_init<std::shared_ptr<Variable>>(quadruple.arg1, [this](auto& var){ ++usage[var]; });
        if_init<std::shared_ptr<Variable>>(quadruple.arg2, [this](auto& var){ ++usage[var]; });
    } else if(!quadruple.is_if() && quadruple.is_if_false()){
        if(quadruple.result && quadruple.op != mtac::Operator::CALL){
            assigns.emplace(std::make_pair(quadruple.result, std::ref(quadruple)));
        }

        if(quadruple.op == mtac::Operator::ASSIGN){
            if_type<std::shared_ptr<Variable>>(quadruple.arg1, [&quadruple, this](auto& var){
                //We only duplicate the math operation if the variable is used once to not add overhead
                if(!quadruple.result->type()->is_array() && var->type() != STRING && usage[var] == 1 && assigns.find(var) != assigns.end()){
                    auto& assign = assigns.at(var).get();
                    quadruple.op = assign.op;
                    quadruple.arg1 = assign.arg1;
                    quadruple.arg2 = assign.arg2;

                    optimized = true;
                }
            });
        }
    }
}
