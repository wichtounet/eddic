//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "mtac/RemoveAssign.hpp"
#include "mtac/OptimizerUtils.hpp"

#include "Variable.hpp"

using namespace eddic;

void mtac::RemoveAssign::collect(mtac::Argument* arg){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
        used.insert(*ptr);
    }
}

void mtac::RemoveAssign::collect_optional(boost::optional<mtac::Argument>& arg){
    if(arg){
        collect(&*arg);
    }
}

bool mtac::RemoveAssign::operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
    if(pass == mtac::Pass::DATA_MINING){
        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);

        return true;
    } else {
        //These operators are not erasing result
        if(quadruple->op == mtac::Operator::DOT_ASSIGN || quadruple->op == mtac::Operator::ARRAY_ASSIGN){
            return true;
        }

        //x = x is never useful
        if((quadruple->op == mtac::Operator::ASSIGN || quadruple->op == mtac::Operator::FASSIGN) && *quadruple->arg1 == quadruple->result){
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

bool mtac::RemoveAssign::operator()(std::shared_ptr<mtac::Param>& param){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&param->arg);
    }

    return true;
}

bool mtac::RemoveAssign::operator()(std::shared_ptr<mtac::IfFalse>& ifFalse){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&ifFalse->arg1);
        collect_optional(ifFalse->arg2);
    }

    return true;
}

bool mtac::RemoveAssign::operator()(std::shared_ptr<mtac::If>& if_){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&if_->arg1);
        collect_optional(if_->arg2);
    }

    return true;
}
