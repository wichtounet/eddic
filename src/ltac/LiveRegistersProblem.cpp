//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "VisitorUtils.hpp"

#include "ltac/LiveRegistersProblem.hpp"
#include "ltac/Statement.hpp"

using namespace eddic;

typedef ltac::LiveRegistersProblem::ProblemDomain ProblemDomain;

std::ostream& ltac::operator<<(std::ostream& stream, ltac::LiveRegisterValues& value){
    stream << "set{";

    for(auto& v : value.registers){
        stream << v << ", ";
    }
    
    for(auto& v : value.float_registers){
        stream << v << ", ";
    }

    return stream << "}";
}

ProblemDomain ltac::LiveRegistersProblem::Boundary(std::shared_ptr<mtac::Function> /*function*/){
    auto value = default_element();
    return value;
}

ProblemDomain ltac::LiveRegistersProblem::Init(std::shared_ptr<mtac::Function> /*function*/){
    auto value = default_element();
    return value;
}

ProblemDomain ltac::LiveRegistersProblem::meet(ProblemDomain& in, ProblemDomain& out){
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
            if(ltac::erase_result(instruction->op)){
                set_dead(*instruction->arg1);
            } else {
                set_live_opt(instruction->arg1);
            }

            set_live_opt(instruction->arg2);
            set_live_opt(instruction->arg3);
        }
    }

    template<typename T>
    void operator()(T&){
        //Nothing to do
    }
};

} //End of anonymous namespace

ProblemDomain ltac::LiveRegistersProblem::transfer(std::shared_ptr<mtac::BasicBlock> /*basic_block*/, ltac::Statement& statement, ProblemDomain& in){
    auto out = in;
    
    LivenessCollector collector(out);
    visit(collector, statement);

    return out;
}

bool ltac::LiveRegistersProblem::optimize(ltac::Statement& /*statement*/, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> /*results*/){
    //This analysis is only made to gather information, not to optimize anything
    throw "Unimplemented";
}
