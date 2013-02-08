//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
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

    void collect(mtac::Quadruple& quadruple){
        if(!mtac::erase_result(quadruple.op)){
            inc_usage(quadruple.result);
        }

        collect_optional(quadruple.arg1);
        collect_optional(quadruple.arg2);
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

    bool collect(mtac::Quadruple& quadruple){
        return quadruple.result == var || collect_optional(quadruple.arg1) || collect_optional(quadruple.arg2);
    }
};

template<typename Container>
mtac::Usage compute_read_usage(Container& loop){
    mtac::Usage usage;
    VariableReadCollector collector(usage);

    for(auto& bb : loop){
        for(auto& quadruple : bb->statements){
           collector.collect(quadruple);
        }
    }

    return usage;
}

template<typename Container>
mtac::Usage compute_write_usage(Container& loop){
    mtac::Usage usage;

    for(auto& bb : loop){
        for(auto& quadruple : bb){
            if(mtac::erase_result(quadruple.op)){
                ++(usage.written[quadruple.result]);
                ++(usage.written[quadruple.secondary]);
            } 
        }
    }

    return usage;
}

struct VariableUsageCollector {
    mtac::VariableUsage& usage;
    int depth_factor;
    int current_depth;

    VariableUsageCollector(mtac::VariableUsage& usage, int depth_factor) : usage(usage), depth_factor(depth_factor) {}

    void inc_usage(std::shared_ptr<Variable> variable){
        usage[variable] += pow(depth_factor, current_depth);
    }

    template<typename T>
    void collect_optional(T& opt){
        if(opt){
            if(auto* variablePtr = boost::get<std::shared_ptr<Variable>>(&*opt)){
                inc_usage(*variablePtr);
            }
        }
    }

    void collect(mtac::Quadruple& quadruple){
        current_depth = quadruple.depth;

        inc_usage(quadruple.result);
        collect_optional(quadruple.arg1);
        collect_optional(quadruple.arg2);
    }
};

} //end of anonymous namespace

mtac::Usage mtac::compute_read_usage(mtac::Loop& loop){
    return ::compute_read_usage(loop);
}

mtac::Usage mtac::compute_read_usage(mtac::Function& function){
    return ::compute_read_usage(function);
}

mtac::Usage mtac::compute_write_usage(mtac::Loop& loop){
    return ::compute_write_usage(loop);
}

mtac::Usage mtac::compute_write_usage(mtac::Function& function){
    return ::compute_write_usage(function);
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

mtac::VariableUsage mtac::compute_variable_usage(mtac::Function& function){
    return compute_variable_usage_with_depth(function, 1);
}

mtac::VariableUsage mtac::compute_variable_usage_with_depth(mtac::Function& function, int factor){
    mtac::VariableUsage usage;

    VariableUsageCollector collector(usage, factor);

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            collector.collect(quadruple);
        }
    }

    return usage;
}
