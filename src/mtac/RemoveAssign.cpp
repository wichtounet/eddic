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

void mtac::RemoveAssign::operator()(std::shared_ptr<mtac::Quadruple>& quadruple){
    if(pass == mtac::Pass::DATA_MINING){
        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);
    } else {
        //These operators are not erasing result
        if(quadruple->op == mtac::Operator::DOT_ASSIGN || quadruple->op == mtac::Operator::ARRAY_ASSIGN){
            return;
        }

        //x = x is never useful
        if((quadruple->op == mtac::Operator::ASSIGN || quadruple->op == mtac::Operator::FASSIGN) && *quadruple->arg1 == quadruple->result){
            quadruple->op = mtac::Operator::NOP;
            optimized = true;
            return;
        }

        if(used.find(quadruple->result) == used.end()){
            if(quadruple->result){
                //The other kind of variables can be used in other basic block
                if(quadruple->result->position().isTemporary()){
                    quadruple->op = mtac::Operator::NOP;
                    optimized = true;
                }
            }
        }
    }
}

void mtac::RemoveAssign::operator()(std::shared_ptr<mtac::Param>& param){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&param->arg);
    }
}

void mtac::RemoveAssign::operator()(std::shared_ptr<mtac::IfFalse>& ifFalse){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&ifFalse->arg1);
        collect_optional(ifFalse->arg2);
    }
}

void mtac::RemoveAssign::operator()(std::shared_ptr<mtac::If>& if_){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&if_->arg1);
        collect_optional(if_->arg2);
    }
}
