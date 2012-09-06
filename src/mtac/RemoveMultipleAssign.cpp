//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"

#include "mtac/RemoveMultipleAssign.hpp"
#include "mtac/OptimizerUtils.hpp"
#include "mtac/Utils.hpp"

#include "mtac/Quadruple.hpp"
#include "mtac/IfFalse.hpp"
#include "mtac/If.hpp"
#include "mtac/Param.hpp"

using namespace eddic;

namespace {

void collect(mtac::Argument* arg, std::unordered_set<std::shared_ptr<Variable>>& used){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*arg)){
        used.insert(*ptr);
    }
}

void collect(boost::optional<mtac::Argument>& arg, std::unordered_set<std::shared_ptr<Variable>>& used){
    if(arg){
        collect(&*arg, used);
    }
}

} //end of anonymous namespace

void mtac::RemoveMultipleAssign::operator()(std::shared_ptr<mtac::Quadruple> quadruple){
    if(pass == mtac::Pass::DATA_MINING){
        collect(quadruple->arg1, used);
        collect(quadruple->arg2, used);
        
        if(!mtac::erase_result(quadruple->op)){
            return;
        }
        
        if(quadruple->result){
            //If the variable have not been used since the last assign
            if(used.find(quadruple->result) == used.end() && lastAssign.find(quadruple->result) != lastAssign.end()){
                //Mark the last assign as useless
                removed.insert(lastAssign[quadruple->result]);
            }

            used.erase(quadruple->result);
            lastAssign[quadruple->result] = quadruple;
        }
    } else {
        if(removed.find(quadruple) != removed.end()){
            optimized = true;
            quadruple->op = mtac::Operator::NOP;
            quadruple->result = nullptr;
            quadruple->arg1.reset();
            quadruple->arg2.reset();
        }
    }
}

void mtac::RemoveMultipleAssign::operator()(std::shared_ptr<mtac::Param> param){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&param->arg, used);
    }
}

void mtac::RemoveMultipleAssign::operator()(std::shared_ptr<mtac::IfFalse> ifFalse){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&ifFalse->arg1, used);
        collect(ifFalse->arg2, used);
    }
}

void mtac::RemoveMultipleAssign::operator()(std::shared_ptr<mtac::If> if_){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&if_->arg1, used);
        collect(if_->arg2, used);
    }
}
