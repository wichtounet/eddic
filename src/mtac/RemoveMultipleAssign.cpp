//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/RemoveMultipleAssign.hpp"
#include "mtac/OptimizerUtils.hpp"

#include "Variable.hpp"

using namespace eddic;

void tac::RemoveMultipleAssign::collect(tac::Argument* arg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*arg)){
        used.insert(*ptr);
    }
}

void tac::RemoveMultipleAssign::collect(boost::optional<tac::Argument>& arg){
    if(arg){
        collect(&*arg);
    }
}

bool tac::RemoveMultipleAssign::operator()(std::shared_ptr<tac::Quadruple>& quadruple){
    if(pass == tac::Pass::DATA_MINING){
        collect(quadruple->arg1);
        collect(quadruple->arg2);
        
        //These operators are not erasing result
        if(quadruple->op == tac::Operator::DOT_ASSIGN || quadruple->op == tac::Operator::ARRAY_ASSIGN){
            return true;
        }
        
        if(quadruple->result){
            //If the variable have not been used since the last assign
            if(used.find(quadruple->result) == used.end() && lastAssign.find(quadruple->result) != lastAssign.end()){
                //Mark the last assign as useless
                removed.insert(lastAssign[quadruple->result]);

                optimized = true;
            }

            used.erase(quadruple->result);
            lastAssign[quadruple->result] = quadruple;
        }
        
        return true;
    } else {
        //keep if not found
        return removed.find(quadruple) == removed.end();
    }
}

bool tac::RemoveMultipleAssign::operator()(std::shared_ptr<tac::Param>& param){
    if(pass == tac::Pass::DATA_MINING){
        collect(&param->arg);
    }

    return true;
}

bool tac::RemoveMultipleAssign::operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
    if(pass == tac::Pass::DATA_MINING){
        collect(&ifFalse->arg1);
        collect(ifFalse->arg2);
    }

    return true;
}

bool tac::RemoveMultipleAssign::operator()(std::shared_ptr<tac::If>& if_){
    if(pass == tac::Pass::DATA_MINING){
        collect(&if_->arg1);
        collect(if_->arg2);
    }

    return true;
}
