//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Variable.hpp"
#include "Type.hpp"

#include "mtac/LiveVariableAnalysisProblem.hpp"
#include "mtac/Utils.hpp"

using namespace eddic;

typedef mtac::LiveVariableAnalysisProblem::ProblemDomain ProblemDomain;

std::ostream& operator<<(std::ostream& stream, mtac::LiveVariableValues& value){
    stream << "set{";

    for(auto& v : value){
        stream << v->name() << ", ";
    }

    return stream << "}";
}

mtac::LiveVariableAnalysisProblem::LiveVariableAnalysisProblem(){
    pointer_escaped = std::make_shared<Values>();
}

void mtac::LiveVariableAnalysisProblem::Gather(std::shared_ptr<mtac::Function> function){
    for(auto& block : function->getBasicBlocks()){
        for(auto& statement : block->statements){
            //Passing a variable as param by address escape its liveness
            if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
                auto& param = *ptr;

                if(param->address){
                    if(mtac::isVariable(param->arg)){
                        escaped_variables.insert(boost::get<std::shared_ptr<Variable>>(param->arg));
                    }
                }
            } 
            //Taking the address of a variable escape its liveness
            else if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
                auto& quadruple = *ptr;

                if(quadruple->op == mtac::Operator::PASSIGN){
                    if(quadruple->arg1 && mtac::isVariable(*quadruple->arg1)){
                        auto var = boost::get<std::shared_ptr<Variable>>(*quadruple->arg1);
                        escaped_variables.insert(var);
                        pointer_escaped->insert(var);
                    }
                } else if(quadruple->op == mtac::Operator::DOT_PASSIGN){
                    if(quadruple->arg2 && mtac::isVariable(*quadruple->arg2)){
                        auto var = boost::get<std::shared_ptr<Variable>>(*quadruple->arg2);
                        escaped_variables.insert(var);
                        pointer_escaped->insert(var);
                    }
                }
            }
        }
    }
}

ProblemDomain mtac::LiveVariableAnalysisProblem::Boundary(std::shared_ptr<mtac::Function> /*function*/){
    auto value = default_element();

    value.values().pointer_escaped = pointer_escaped;

    return value;
}

ProblemDomain mtac::LiveVariableAnalysisProblem::Init(std::shared_ptr<mtac::Function> /*function*/){
    auto value = default_element();

    value.values().pointer_escaped = pointer_escaped;

    return value;
}

ProblemDomain mtac::LiveVariableAnalysisProblem::meet(ProblemDomain& out, ProblemDomain& in){
    if(out.top()){
        return in;
    } else if(in.top()){
        return out;
    }

    typename ProblemDomain::Values values;
    ProblemDomain result(values);
    result.values().pointer_escaped = pointer_escaped;

    for(auto& value : in.values()){
        result.values().insert(value);
    }
    
    for(auto& value : out.values()){
        result.values().insert(value);
    }

    return result;
}

template<typename Arg, typename Values>
void update(Arg& arg, Values& values){
    if(mtac::isVariable(arg)){
        auto var = boost::get<std::shared_ptr<Variable>>(arg);
        values.insert(var);
    }
}

template<typename Arg, typename Values>
void update_optional(Arg& arg, Values& values){
    if(arg){
        update(*arg, values);
    }
}

ProblemDomain mtac::LiveVariableAnalysisProblem::transfer(std::shared_ptr<mtac::BasicBlock>/* basic_block*/, mtac::Statement& statement, ProblemDomain& out){
    auto in = out;

    if(auto* ptr = boost::get<std::shared_ptr<mtac::Quadruple>>(&statement)){
        auto quadruple = *ptr;
        
        if(mtac::erase_result(quadruple->op)){
            in.values().erase(quadruple->result);
        } else {
            in.values().insert(quadruple->result);
        }

        update_optional((*ptr)->arg1, in.values());
        update_optional((*ptr)->arg2, in.values());
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::Param>>(&statement)){
        update((*ptr)->arg, in.values());
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::IfFalse>>(&statement)){
        update((*ptr)->arg1, in.values());
        update_optional((*ptr)->arg2, in.values());
    } else if(auto* ptr = boost::get<std::shared_ptr<mtac::If>>(&statement)){
        update((*ptr)->arg1, in.values());
        update_optional((*ptr)->arg2, in.values());
    }

    for(auto& escaped_var : escaped_variables){
        in.values().insert(escaped_var);
    }

    return in;
}

bool mtac::LiveVariableAnalysisProblem::optimize(mtac::Statement& /*statement*/, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>>& /*global_results*/){
    //This analysis is only made to gather information, not to optimize anything
    throw "Unimplemented";
}
