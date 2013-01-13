//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"
#include "Type.hpp"

#include "mtac/MathPropagation.hpp"
#include "mtac/OptimizerUtils.hpp"

using namespace eddic;

void mtac::MathPropagation::clear(){
    optimized = false;
    assigns.clear();
    usage.clear();
}

void mtac::MathPropagation::collect(mtac::Argument* arg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
        usage[*ptr] += 1;
    }
}

void mtac::MathPropagation::collect(boost::optional<mtac::Argument>& arg){
    if(arg){
        collect(&*arg);
    }
}

void mtac::MathPropagation::operator()(mtac::Quadruple& quadruple){
    if(pass == mtac::Pass::DATA_MINING){
        collect(quadruple.arg1);
        collect(quadruple.arg2);
    } else if(!quadruple.is_if() && quadruple.is_if_false()){
        if(quadruple.result && quadruple.op != mtac::Operator::CALL){
            assigns.emplace(std::make_pair(quadruple.result, std::ref(quadruple)));
        }

        if(quadruple.op == mtac::Operator::ASSIGN){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple.arg1)){
                //We only duplicate the math operation if the variable is used once to not add overhead
                if(!quadruple.result->type()->is_array() && (*ptr)->type() != STRING && usage[*ptr] == 1 && assigns.find(*ptr) != assigns.end()){
                    auto& assign = assigns.at(*ptr).get();
                    quadruple.op = assign.op;
                    quadruple.arg1 = assign.arg1;
                    quadruple.arg2 = assign.arg2;

                    optimized = true;
                }
            }
        }
    }
}
