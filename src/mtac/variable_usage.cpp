//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "VisitorUtils.hpp"

#include "mtac/variable_usage.hpp"
#include "mtac/Loop.hpp"
#include "mtac/Function.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

namespace {

struct VariableReadCollector {
    mtac::Usage& usage;

    VariableReadCollector(mtac::Usage& usage) : usage(usage) {}

    void inc_usage(std::shared_ptr<Variable> variable){
        ++usage.read[variable];
    }

    template<typename T>
    void collect_optional(T& opt){
        if(opt){
            if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*opt)){
                inc_usage(*variablePtr);
            }
        }
    }

    void collect(std::shared_ptr<mtac::Quadruple> quadruple){
        if(!mtac::erase_result(quadruple->op)){
            inc_usage(quadruple->result);
        }

        collect_optional(quadruple->arg1);
        collect_optional(quadruple->arg2);
    }
};

struct UsageCollector {
    std::shared_ptr<Variable> var;

    UsageCollector(std::shared_ptr<Variable> var) : var(var) {}

    template<typename T>
    bool collect_optional(T& opt){
        if(opt){
            if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*opt)){
                return *variablePtr == var;
            }
        }

        return false;
    }

    bool collect(std::shared_ptr<mtac::Quadruple> quadruple){
        return quadruple->result == var || collect_optional(quadruple->arg1) || collect_optional(quadruple->arg2);
    }
};

} //end of anonymous namespace

mtac::Usage mtac::compute_read_usage(mtac::Loop& loop){
    mtac::Usage usage;
    VariableReadCollector collector(usage);

    for(auto& bb : loop){
        for(auto& quadruple : bb->statements){
           collector.collect(quadruple);
        }
    }

    return usage;
}

mtac::Usage mtac::compute_write_usage(mtac::Loop& loop){
    mtac::Usage usage;

    for(auto& bb : loop){
        for(auto& quadruple : bb->statements){
            if(mtac::erase_result(quadruple->op)){
                ++(usage.written[quadruple->result]);
                ++(usage.written[quadruple->secondary]);
            } 
        }
    }

    return usage;
}

bool mtac::use_variable(mtac::basic_block_p bb, std::shared_ptr<Variable> var){
    UsageCollector collector(var);

    for(auto& statement : bb->statements){
        if(collector.collect(statement)){
            return true;
        }
    }

    return false;
}
