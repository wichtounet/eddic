//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "VisitorUtils.hpp"

#include "ltac/Statement.hpp"
#include "ltac/LiveRegistersProblem.hpp"

using namespace eddic;

typedef ltac::LivePseudoRegistersProblem::ProblemDomain ProblemDomain;
    
ProblemDomain ltac::LivePseudoRegistersProblem::Boundary(mtac::function_p /*function*/){
    auto value = default_element();
    return value;
}

ProblemDomain ltac::LivePseudoRegistersProblem::Init(mtac::function_p /*function*/){
    auto value = default_element();
    return value;
}

ProblemDomain ltac::LivePseudoRegistersProblem::meet(ProblemDomain& in, ProblemDomain& out){
    if(out.top()){
        return in;
    } else if(in.top()){
        return out;
    }

    typename ProblemDomain::Values values;
    ProblemDomain result(values);

    for(auto& value : in.values().registers){
        result.values().insert(value);
    }
    
    for(auto& value : in.values().float_registers){
        result.values().insert(value);
    }
    
    for(auto& value : out.values().registers){
        result.values().insert(value);
    }
    
    for(auto& value : out.values().float_registers){
        result.values().insert(value);
    }

    return result;
}

namespace {

struct LivenessCollector : public boost::static_visitor<> {
    ProblemDomain& in;

    LivenessCollector(ProblemDomain& in) : in(in) {}

    template<typename Arg>
    inline void set_live(Arg& arg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&arg)){
            in.values().insert(*ptr);
        } else if(auto* ptr = boost::get<ltac::PseudoFloatRegister>(&arg)){
            in.values().insert(*ptr);
        } else if(auto* ptr = boost::get<ltac::Address>(&arg)){
            set_live_opt(ptr->base_register);
            set_live_opt(ptr->scaled_register);
        }
    }
    
    template<typename Arg>
    inline void set_dead(Arg& arg){
        if(auto* ptr = boost::get<ltac::PseudoRegister>(&arg)){
            in.values().erase(*ptr);
        } else if(auto* ptr = boost::get<ltac::PseudoFloatRegister>(&arg)){
            in.values().erase(*ptr);
        }     
    }

    template<typename Arg>
    inline void set_live_opt(Arg& arg){
        if(arg){
            set_live(*arg);
        }
    }

    void operator()(std::shared_ptr<ltac::Instruction> instruction){
        if(instruction->op != ltac::Operator::NOP){
            if(ltac::erase_result_complete(instruction->op)){
                if(auto* ptr = boost::get<ltac::Address>(&*instruction->arg1)){
                    set_live_opt(ptr->base_register);
                    set_live_opt(ptr->scaled_register);
                } else {
                    set_dead(*instruction->arg1);
                }
            } else {
                set_live_opt(instruction->arg1);
            }

            set_live_opt(instruction->arg2);
            set_live_opt(instruction->arg3);
        }

        for(auto& reg : instruction->uses){
            in.values().insert(reg);
        }
        
        for(auto& reg : instruction->float_uses){
            in.values().insert(reg);
        }
    }
    
    void operator()(std::shared_ptr<ltac::Jump> instruction){
        for(auto& reg : instruction->uses){
            in.values().insert(reg);
        }
        
        for(auto& reg : instruction->float_uses){
            in.values().insert(reg);
        }
        
        for(auto& reg : instruction->kills){
            in.values().erase(reg);
        }
        
        for(auto& reg : instruction->float_kills){
            in.values().erase(reg);
        }
    }

    template<typename T>
    void operator()(T&){
        //Nothing to do
    }
};

} //End of anonymous namespace

ProblemDomain ltac::LivePseudoRegistersProblem::transfer(mtac::basic_block_p /*basic_block*/, ltac::Statement& statement, ProblemDomain& in){
    auto out = in;
    
    LivenessCollector collector(out);
    visit(collector, statement);

    return out;
}

bool ltac::LivePseudoRegistersProblem::optimize(ltac::Statement& /*statement*/, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> /*results*/){
    //This analysis is only made to gather information, not to optimize anything
    throw "Unimplemented";
}
