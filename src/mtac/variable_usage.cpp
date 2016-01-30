//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "VisitorUtils.hpp"

#include "mtac/variable_usage.hpp"
#include "mtac/loop.hpp"
#include "mtac/Function.hpp"
#include "mtac/Utils.hpp"
#include "mtac/Quadruple.hpp"

using namespace eddic;

namespace {

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

    for(auto& bb : loop){
        for(auto& quadruple : bb->statements){
            if(!mtac::erase_result(quadruple.op)){
                ++usage.read[quadruple.result];
            }

            if_init<std::shared_ptr<Variable>>(quadruple.arg1, [&usage](std::shared_ptr<Variable>& var){++usage.read[var];});
            if_init<std::shared_ptr<Variable>>(quadruple.arg2, [&usage](std::shared_ptr<Variable>& var){++usage.read[var];});
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

            if(quadruple.op == mtac::Operator::CALL){
                ++(usage.written[quadruple.result]);
                ++(usage.written[quadruple.secondary]);
            }
        }
    }

    return usage;
}

} //end of anonymous namespace

mtac::Usage mtac::compute_read_usage(mtac::loop& loop){
    return ::compute_read_usage(loop);
}

mtac::Usage mtac::compute_read_usage(mtac::Function& function){
    return ::compute_read_usage(function);
}

mtac::Usage mtac::compute_write_usage(mtac::loop& loop){
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

mtac::VariableUsage mtac::compute_variable_usage_with_depth(mtac::Function& function, int depth_factor){
    mtac::VariableUsage usage;

    for(auto& block : function){
        for(auto& quadruple : block->statements){
            usage[quadruple.result] += pow(depth_factor, block->depth);
            if_init<std::shared_ptr<Variable>>(quadruple.arg1, [&usage, depth_factor, &block](std::shared_ptr<Variable>& var){ usage[var] += pow(depth_factor, block->depth); });
            if_init<std::shared_ptr<Variable>>(quadruple.arg2, [&usage, depth_factor, &block](std::shared_ptr<Variable>& var){ usage[var] += pow(depth_factor, block->depth); });
        }
    }

    return std::move(usage);
}
