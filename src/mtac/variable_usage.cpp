//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "VisitorUtils.hpp"

#include "mtac/variable_usage.hpp"
#include "mtac/Loop.hpp"
#include "mtac/Statement.hpp"
#include "mtac/Function.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

namespace {

struct VariableReadCollector : public boost::static_visitor<> {
    mtac::Usage& usage;

    VariableReadCollector(mtac::Usage& usage) : usage(usage) {}

    void inc_usage(std::shared_ptr<Variable> variable){
        ++usage.read[variable];
    }

    template<typename T>
    void collect(T& arg){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&arg)){
            inc_usage(*variablePtr);
        }
    }

    template<typename T>
    void collect_optional(T& opt){
        if(opt){
            collect(*opt);
        }
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        if(!mtac::erase_result(quadruple->op)){
            inc_usage(quadruple->result);
        }

        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::If> if_){
        collect(if_->arg1);
        collect_optional(if_->arg2);
    }
    
    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        collect(if_false->arg1);
        collect_optional(if_false->arg2);
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

struct UsageCollector : public boost::static_visitor<bool> {
    std::shared_ptr<Variable> var;

    UsageCollector(std::shared_ptr<Variable> var) : var(var) {}

    template<typename T>
    bool collect(T& arg){
        if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&arg)){
            return *variablePtr == var;
        }

        return false;
    }

    template<typename T>
    bool collect_optional(T& opt){
        if(opt){
            return collect(*opt);
        }

        return false;
    }

    bool operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        return quadruple->result == var || collect_optional(quadruple->arg1) || collect_optional(quadruple->arg2);
    }
    
    bool operator()(std::shared_ptr<mtac::If> if_){
        return collect(if_->arg1) || collect_optional(if_->arg2);
    }
    
    bool operator()(std::shared_ptr<mtac::IfFalse> if_false){
        return collect(if_false->arg1) || collect_optional(if_false->arg2);
    }

    template<typename T>
    bool operator()(T&){
        return false;
    }
};

} //end of anonymous namespace

mtac::Usage mtac::compute_read_usage(std::shared_ptr<mtac::Loop> loop){
    mtac::Usage usage;
    VariableReadCollector collector(usage);

    for(auto& bb : loop){
        visit_each(collector, bb->statements);
    }

    return usage;
}

mtac::Usage mtac::compute_write_usage(std::shared_ptr<mtac::Loop> loop){
    mtac::Usage usage;

    for(auto& bb : loop){
        for(auto& statement : bb->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;
                if(mtac::erase_result(quadruple->op)){
                    ++(usage.written[quadruple->result]);
                } 
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                auto call = *ptr;

                if(call->return_){
                    ++(usage.written[call->return_]);
                }
                
                if(call->return2_){
                    ++(usage.written[call->return2_]);
                }
            }
        }
    }

    return usage;
}

bool mtac::use_variable(mtac::basic_block_p bb, std::shared_ptr<Variable> var){
    UsageCollector collector(var);

    for(auto& statement : bb->statements){
        if(visit(collector, statement)){
            return true;
        }
    }

    return false;
}
