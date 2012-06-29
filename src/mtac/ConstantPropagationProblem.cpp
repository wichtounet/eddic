//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "Type.hpp"

#include "mtac/ConstantPropagationProblem.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

typedef mtac::ConstantPropagationProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::ConstantPropagationProblem::meet(ProblemDomain& in, ProblemDomain& out){
    auto result = mtac::intersection_meet(in, out);

    //Remove all the temporary
    for(auto it = std::begin(result.values()); it != std::end(result.values());){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&it->second)){
            auto variable = *ptr;

            if (variable->position().isTemporary()){
                it = result.values().erase(it);
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    return result;
}

ProblemDomain mtac::ConstantPropagationProblem::transfer(std::shared_ptr<mtac::BasicBlock>/* basic_block*/, mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    //Quadruple affects variable
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;

        if(quadruple->op == mtac::Operator::ASSIGN || quadruple->op == mtac::Operator::FASSIGN){
            if(auto* ptr = boost::get<int>(&*quadruple->arg1)){
                out[quadruple->result] = *ptr;
            } else if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&*quadruple->arg1)){
                if(*ptr != quadruple->result){
                    out[quadruple->result] = *ptr;
                }
            } else if(auto* ptr = boost::get<double>(&*quadruple->arg1)){
                out[quadruple->result] = *ptr;
            } else if(auto* ptr = boost::get<std::string>(&*quadruple->arg1)){
                out[quadruple->result] = *ptr;
            } else {
                //The result is not constant at this point
                out.erase(quadruple->result);
            }
        } else {
            auto op = quadruple->op;

            if(mtac::erase_result(op)){
                //The result is not constant at this point
                out.erase(quadruple->result);
    
                //Cancel the copy of the variable erased
                for(auto it = std::begin(out.values()); it != std::end(out.values());){
                    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&it->second)){
                        auto variable = *ptr;

                        if (variable == quadruple->result){
                            it = out.values().erase(it);
                        } else {
                            ++it;
                        }
                    } else {
                        ++it;
                    }
                }
            }
        }
    }
    //Passing a variable by pointer erases its value
    else if (auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
        auto param = *ptr;

        if(param->address){
            auto variable = boost::get<std::shared_ptr<Variable>>(param->arg);

            //Impossible to know if the variable is modified or not, consider it modified
            out.erase(variable);
        }
    }

    return out;
}

bool optimize_arg(mtac::Argument* arg, ProblemDomain& results){
    if(auto* ptr = boost::get<std::shared_ptr<Variable>>(arg)){
        if(results.find(*ptr) != results.end()){
            *arg = results[*ptr];
            return true;
        }
    }

    return false;
}

bool optimize_optional(boost::optional<mtac::Argument>& arg, ProblemDomain& results){
    if(arg){
        return optimize_arg(&*arg, results);
    }

    return false;
}

bool mtac::ConstantPropagationProblem::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& global_results){
    auto& results = global_results->IN_S[statement];

    bool changes = false;

    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto& quadruple = *ptr;

        //Do not replace a variable by a constant when used in offset
        if(quadruple->op != mtac::Operator::ARRAY && quadruple->op != mtac::Operator::DOT){
            changes |= optimize_optional(quadruple->arg1, results);
        }
        
        if(quadruple->op != mtac::Operator::ARRAY_PASSIGN){
            changes |= optimize_optional(quadruple->arg2, results);
        }

        if(!mtac::erase_result(quadruple->op) && quadruple->result){
            if(results.find(quadruple->result) != results.end()){
                if(mtac::isVariable(results[quadruple->result])){
                    auto var = boost::get<std::shared_ptr<Variable>>(results[quadruple->result]);

                    if(!var->position().isTemporary()){
                        quadruple->result = var;
                    }
                }
            }
        }
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
        auto& param = *ptr;

        if(!param->address){
            changes |= optimize_arg(&param->arg, results);
        }
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
        auto& ifFalse = *ptr;
        
        changes |= optimize_arg(&ifFalse->arg1, results);
        changes |= optimize_optional(ifFalse->arg2, results);
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
        auto& if_ = *ptr;
        
        changes |= optimize_arg(&if_->arg1, results);
        changes |= optimize_optional(if_->arg2, results);
    }

    return changes;
}
