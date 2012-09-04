//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "Type.hpp"
#include "VisitorUtils.hpp"

#include "mtac/ConstantPropagationProblem.hpp"
#include "mtac/Utils.hpp"
#include "mtac/GlobalOptimizations.hpp"
#include "mtac/LiveVariableAnalysisProblem.hpp"

using namespace eddic;

typedef mtac::ConstantPropagationProblem::ProblemDomain ProblemDomain;

ProblemDomain mtac::ConstantPropagationProblem::Boundary(std::shared_ptr<mtac::Function> function){
    pointer_escaped = mtac::escape_analysis(function);

    return default_element();
}

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

namespace {

struct ConstantCollector : public boost::static_visitor<> {
    ProblemDomain& out;
    std::shared_ptr<Variable> var;

    ConstantCollector(ProblemDomain& out, std::shared_ptr<Variable> var) : out(out), var(var) {}

    void operator()(int value){
        out[var] = value;
        std::cout << var->name() << ":" << value << std::endl;
    }
    
    void operator()(const std::string& value){
        out[var] = value;
    }
    
    void operator()(double value){
        out[var] = value;
    }
    
    void operator()(std::shared_ptr<Variable> variable){
        if(variable != var){
            out[var] = variable;
        }
    }

    template<typename T>
    void operator()(T&){
        out.erase(var);
    }
};

} //end of anonymous namespace

ProblemDomain mtac::ConstantPropagationProblem::transfer(std::shared_ptr<mtac::BasicBlock>/* basic_block*/, mtac::Statement& statement, ProblemDomain& in){
    auto out = in;

    //Quadruple affects variable
    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;

        if(quadruple->op == mtac::Operator::NOP){
            return out;
        }

        if(quadruple->op == mtac::Operator::ASSIGN || quadruple->op == mtac::Operator::FASSIGN){
            ConstantCollector collector(out, quadruple->result);
            visit(collector, *quadruple->arg1);
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

namespace {

struct ConstantOptimizer : public boost::static_visitor<> {
    mtac::Domain<mtac::ConstantPropagationValues>& results;
    mtac::EscapedVariables& pointer_escaped;
    bool changes = false;

    ConstantOptimizer(mtac::Domain<mtac::ConstantPropagationValues>& results, mtac::EscapedVariables& pointer_escaped) : results(results), pointer_escaped(pointer_escaped) {}

    bool optimize_arg(mtac::Argument& arg){
        if(auto* ptr = boost::get<std::shared_ptr<Variable>>(&arg)){
            if(results.find(*ptr) != results.end() && pointer_escaped->find(*ptr) == pointer_escaped->end()){
                arg = results[*ptr];
                return true;
            }
        }

        return false;
    }

    bool optimize_optional(boost::optional<mtac::Argument>& arg){
        if(arg){
            return optimize_arg(*arg);
        }

        return false;
    }

    void operator()(std::shared_ptr<mtac::Quadruple> quadruple){
        //Do not replace a variable by a constant when used in offset
        if(quadruple->op != mtac::Operator::PDOT && quadruple->op != mtac::Operator::DOT && quadruple->op != mtac::Operator::PASSIGN){
            changes |= optimize_optional(quadruple->arg1);
        }
        
        if(quadruple->op != mtac::Operator::DOT_PASSIGN){
            changes |= optimize_optional(quadruple->arg2);
        }

        if(!mtac::erase_result(quadruple->op) && quadruple->result && quadruple->op != mtac::Operator::DOT_ASSIGN){
            if(results.find(quadruple->result) != results.end()){
                if(mtac::isVariable(results[quadruple->result])){
                    auto var = boost::get<std::shared_ptr<Variable>>(results[quadruple->result]);

                    if(!var->position().isTemporary()){
                        quadruple->result = var;
                    }
                }
            }
        }
    }

    void operator()(std::shared_ptr<mtac::Param> param){
        if(!param->address){
            changes |= optimize_arg(param->arg);
        }
    }

    void operator()(std::shared_ptr<mtac::IfFalse> if_false){
        changes |= optimize_arg(if_false->arg1);
        changes |= optimize_optional(if_false->arg2);
    }

    void operator()(std::shared_ptr<mtac::If> if_){
        changes |= optimize_arg(if_->arg1);
        changes |= optimize_optional(if_->arg2);
    }

    template<typename T>
    void operator()(T&){
        //NOP
    }
};

} //end of anonymous namespace

bool mtac::ConstantPropagationProblem::optimize(mtac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> global_results){
    ConstantOptimizer optimizer(global_results->IN_S[statement], pointer_escaped);

    visit(optimizer, statement);

    return optimizer.changes;
}
