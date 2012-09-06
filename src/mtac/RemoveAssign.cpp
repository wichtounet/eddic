//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"

#include "mtac/RemoveAssign.hpp"
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

void collect_optional(boost::optional<mtac::Argument>& arg, std::unordered_set<std::shared_ptr<Variable>>& used){
    if(arg){
        collect(&*arg, used);
    }
}

} //end of anonymous namespace

void mtac::RemoveAssign::operator()(std::shared_ptr<mtac::Quadruple> quadruple){
    if(pass == mtac::Pass::DATA_MINING){
        collect_optional(quadruple->arg1, used);
        collect_optional(quadruple->arg2, used);
    } else {
        if(!mtac::erase_result(quadruple->op)){
            return;
        }

        //x = x is never useful
        if((quadruple->op == mtac::Operator::ASSIGN || quadruple->op == mtac::Operator::FASSIGN) && *quadruple->arg1 == quadruple->result){
            quadruple->op = mtac::Operator::NOP;
            quadruple->arg1.reset();
            quadruple->arg2.reset();
            quadruple->result = nullptr;
            optimized = true;
            return;
        }

        if(used.find(quadruple->result) == used.end()){
            if(quadruple->result){
                //The other kind of variables can be used in other basic block
                if(quadruple->result->position().isTemporary()){
                    quadruple->op = mtac::Operator::NOP;
                    quadruple->arg1.reset();
                    quadruple->arg2.reset();
                    quadruple->result = nullptr;
                    optimized = true;
                }
            }
        }
    }
}

void mtac::RemoveAssign::operator()(std::shared_ptr<mtac::Param> param){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&param->arg, used);
    }
}

void mtac::RemoveAssign::operator()(std::shared_ptr<mtac::IfFalse> ifFalse){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&ifFalse->arg1, used);
        collect_optional(ifFalse->arg2, used);
    }
}

void mtac::RemoveAssign::operator()(std::shared_ptr<mtac::If> if_){
    if(pass == mtac::Pass::DATA_MINING){
        collect(&if_->arg1, used);
        collect_optional(if_->arg2, used);
    }
}
