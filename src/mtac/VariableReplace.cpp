//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "boost_cfg.hpp"
#include <boost/variant.hpp>

#include "mtac/VariableReplace.hpp"
#include "mtac/Statement.hpp"

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

void mtac::VariableReplace::operator()(std::shared_ptr<mtac::Quadruple> quadruple){
    if(clones.find(quadruple->result) != clones.end()){
        quadruple->result = clones[quadruple->result];
    }

    update_usage_optional(quadruple->arg1);
    update_usage_optional(quadruple->arg2);
}

void mtac::VariableReplace::operator()(std::shared_ptr<mtac::Param> param){
    update_usage(param->arg);
}

void mtac::VariableReplace::operator()(std::shared_ptr<mtac::IfFalse> if_false){
    update_usage(if_false->arg1);
    update_usage_optional(if_false->arg2);
}

void mtac::VariableReplace::operator()(std::shared_ptr<mtac::If> if_){
    update_usage(if_->arg1);
    update_usage_optional(if_->arg2);
}

void mtac::VariableReplace::operator()(std::shared_ptr<mtac::Call> call_){
    if(call_->return_ && clones.find(call_->return_) != clones.end()){
        call_->return_ = clones[call_->return_];
    }

    if(call_->return2_ && clones.find(call_->return2_) != clones.end()){
        call_->return2_ = clones[call_->return2_];
    }
}
