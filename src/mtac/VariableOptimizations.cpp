//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <vector>

#include "FunctionContext.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/VariableOptimizations.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

namespace {

bool is_written_once(std::shared_ptr<Variable> variable, std::shared_ptr<mtac::Function> function){
    bool written = false;

    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                if(mtac::erase_result((*ptr)->op) && (*ptr)->result == variable){
                    if(written){
                        return false;
                    }

                    written = true;
                }
            } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Call>>(&statement)){
                if((*ptr)->return_ == variable || (*ptr)->return2_ == variable){
                    if(written){
                        return false;
                    }

                    written = true;
                }
            }
        }
    }

    return true;
}

std::vector<std::shared_ptr<Variable>> get_targets(std::shared_ptr<Variable> variable, std::shared_ptr<mtac::Function> function){
    std::vector<std::shared_ptr<Variable>> targets;
    
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto quadruple = *ptr;

                if(quadruple->op == mtac::Operator::ASSIGN || quadruple->op == mtac::Operator::PASSIGN || quadruple->op == mtac::Operator::FASSIGN){
                    if(auto* var_ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                        if(*var_ptr == variable){
                           targets.push_back(quadruple->result); 
                        }
                    }
                }
            } 
        }
    }

    return targets;
}

struct VariableReplace : public boost::static_visitor<bool> {
    std::shared_ptr<mtac::Function> function;
    std::shared_ptr<Variable> source;
    std::shared_ptr<Variable> target;

    VariableReplace(std::shared_ptr<mtac::Function> function, std::shared_ptr<Variable> source, std::shared_ptr<Variable> target)
        : function(function), source(source), target(target) {}

    inline bool optimize_arg(mtac::Argument& arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
            if(*ptr == source){
                arg = target;
                return true;
            }
        }

        return false;
    }

    inline bool optimize_optional(boost::optional<mtac::Argument>& arg){
        if(arg){
            return optimize_arg(*arg);
        }

        return false;
    }

    bool operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        bool optimized = false;

        if(quadruple->result == source){
            quadruple->result = target;
            optimized = true;
        }
    
        optimized |= optimize_optional(quadruple->arg1);
        optimized |= optimize_optional(quadruple->arg2);

        return optimized;
    }

    bool operator()(std::shared_ptr<mtac::Param> param){
        return optimize_arg(param->arg);
    }

    bool operator()(std::shared_ptr<mtac::IfFalse> if_){
        return optimize_arg(if_->arg1) | optimize_optional(if_->arg2);
    }

    bool operator()(std::shared_ptr<mtac::If> if_){
        return optimize_arg(if_->arg1) | optimize_optional(if_->arg2);
    }

    bool operator()(std::shared_ptr<mtac::Call> call){
        bool optimized = false;

        if(call->return_ == source){
            call->return_ = target;
            optimized = true;
        }
        
        if(call->return2_ == source){
            call->return2_ = target;
            optimized  = true;
        }

        return optimized;
    }

    template<typename T>
    bool operator()(T /*t*/){
        return false;
    }
};

}

bool mtac::remove_aliases(std::shared_ptr<mtac::Function> function){
    bool optimized = false;

    for(auto& pair : function->context->stored_variables()){
        auto var = pair.second;
        auto position = var->position();
        auto type = var->type();

        if((position.isTemporary() || position.isStack()) && (type->is_standard_type() || type->is_pointer())){
            if(is_written_once(var, function)){
                auto targets = get_targets(var, function);

                if(targets.size() == 1){
                    VariableReplace replacer(function, var, targets[0]);

                    for(auto& block : function->getBasicBlocks()){
                        for(auto& statement : block->statements){
                            optimized |= visit(replacer, statement);
                        }
                    }
                }
            }
        }
    }

    return optimized;
}

void mtac::clean_variables(std::shared_ptr<mtac::Function> function){
    auto variable_usage = mtac::compute_variable_usage(function);
    
    std::vector<std::shared_ptr<Variable>> unused;
    for(auto variable_pair : function->context->stored_variables()){
        auto variable = variable_pair.second;

        //Temporary and parameters are not interesting, because they dot not take any space
        if(!variable->position().isParameter() && !variable->position().isParamRegister()){
            if(variable_usage[variable] == 0){
                unused.push_back(variable);
            }
        }
    }

    for(auto& variable : unused){
        function->context->removeVariable(variable->name());
    }
}
