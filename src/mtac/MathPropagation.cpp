//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/MathPropagation.hpp"
#include "mtac/OptimizerUtils.hpp"

#include "Variable.hpp"

using namespace eddic;

void tac::MathPropagation::collect(tac::Argument* arg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
        usage[*ptr] += 1;
    }
}

void tac::MathPropagation::collect(boost::optional<tac::Argument>& arg){
    if(arg){
        collect(&*arg);
    }
}

void tac::MathPropagation::operator()(std::shared_ptr<tac::Quadruple>& quadruple){
    if(pass == tac::Pass::DATA_MINING){
        collect(quadruple->arg1);
        collect(quadruple->arg2);
    } else {
        if(quadruple->result){
            assigns[quadruple->result] = quadruple;
        }

        if(quadruple->op == tac::Operator::ASSIGN){
            if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                //We only duplicate the math operation if the variable is used once to not add overhead
                if(usage[*ptr] == 1 && assigns.find(*ptr) != assigns.end()){
                    auto assign = assigns[*ptr];
                    quadruple->op = assign->op;
                    quadruple->arg1 = assign->arg1;
                    quadruple->arg2 = assign->arg2;

                    optimized = true;
                }
            }
        }
    }
}

void tac::MathPropagation::operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
    if(pass == tac::Pass::DATA_MINING){
        collect(&ifFalse->arg1);
        collect(ifFalse->arg2);
    }
}

void tac::MathPropagation::operator()(std::shared_ptr<tac::If>& if_){
    if(pass == tac::Pass::DATA_MINING){
        collect(&if_->arg1);
        collect(if_->arg2);
    }
}
