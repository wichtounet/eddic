//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "tac/RemoveAssign.hpp"
#include "tac/OptimizerUtils.hpp"

#include "Variable.hpp"

using namespace eddic;

void tac::RemoveAssign::collect(tac::Argument* arg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
        used.insert(*ptr);
    }
}

void tac::RemoveAssign::collect_optional(boost::optional<tac::Argument>& arg){
    if(arg){
        collect(&*arg);
    }
}

bool tac::RemoveAssign::operator()(std::shared_ptr<tac::Quadruple>& quadruple){
    if(pass == tac::Pass::DATA_MINING){
        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);

        return true;
    } else {
        //These operators are not erasing result
        if(quadruple->op == tac::Operator::DOT_ASSIGN || quadruple->op == tac::Operator::ARRAY_ASSIGN){
            return true;
        }

        //x = x is never useful
        if((quadruple->op == tac::Operator::ASSIGN || quadruple->op == tac::Operator::FASSIGN) && *quadruple->arg1 == quadruple->result){
            optimized = true;
            return false;
        }

        if(used.find(quadruple->result) == used.end()){
            if(quadruple->result){
                //The other kind of variables can be used in other basic block
                if(quadruple->result->position().isTemporary()){
                    optimized = true;
                    return false;
                }
            }
        }

        return true;
    }
}

bool tac::RemoveAssign::operator()(std::shared_ptr<tac::Param>& param){
    if(pass == tac::Pass::DATA_MINING){
        collect(&param->arg);
    }

    return true;
}

bool tac::RemoveAssign::operator()(std::shared_ptr<tac::IfFalse>& ifFalse){
    if(pass == tac::Pass::DATA_MINING){
        collect(&ifFalse->arg1);
        collect_optional(ifFalse->arg2);
    }

    return true;
}

bool tac::RemoveAssign::operator()(std::shared_ptr<tac::If>& if_){
    if(pass == tac::Pass::DATA_MINING){
        collect(&if_->arg1);
        collect_optional(if_->arg2);
    }

    return true;
}
