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

typedef ltac::LiveRegistersProblem::ProblemDomain ProblemDomain;
typedef ltac::LivePseudoRegistersProblem::ProblemDomain PseudoProblemDomain;

namespace {

template<typename Reg, typename FloatReg, typename ProblemDomain>
typename std::enable_if<std::is_same<Reg, ltac::PseudoRegister>::value, void>::type collect_jump(std::shared_ptr<ltac::Jump> instruction, ProblemDomain& in){
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

template<typename Reg, typename FloatReg, typename ProblemDomain>
typename std::enable_if<std::is_same<Reg, ltac::Register>::value, void>::type collect_jump(std::shared_ptr<ltac::Jump>, ProblemDomain&){
    //Nothing for now
}

template<typename Reg, typename FloatReg, typename ProblemDomain>
typename std::enable_if<std::is_same<Reg, ltac::PseudoRegister>::value, void>::type collect_instruction(std::shared_ptr<ltac::Instruction> instruction, ProblemDomain& in){
    for(auto& reg : instruction->uses){
        in.values().insert(reg);
    }

    for(auto& reg : instruction->float_uses){
        in.values().insert(reg);
    }
}

template<typename Reg, typename FloatReg, typename ProblemDomain>
typename std::enable_if<std::is_same<Reg, ltac::Register>::value, void>::type collect_instruction(std::shared_ptr<ltac::Instruction>, ProblemDomain&){
    //Nothing for now
}

template<typename Reg, typename FloatReg, typename ProblemDomain>
struct LivenessCollector : public boost::static_visitor<> {
    ProblemDomain& in;

    LivenessCollector(ProblemDomain& in) : in(in) {}

    template<typename Arg>
    inline void set_live(Arg& arg){
        if(auto* ptr = boost::get<Reg>(&arg)){
            in.values().insert(*ptr);
        } else if(auto* ptr = boost::get<FloatReg>(&arg)){
            in.values().insert(*ptr);
        } else if(auto* ptr = boost::get<ltac::Address>(&arg)){
            set_live_opt(ptr->base_register);
            set_live_opt(ptr->scaled_register);
        }
    }
    
    template<typename Arg>
    inline void set_dead(Arg& arg){
        if(auto* ptr = boost::get<Reg>(&arg)){
            in.values().erase(*ptr);
        } else if(auto* ptr = boost::get<FloatReg>(&arg)){
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

        collect_instruction<Reg, FloatReg, ProblemDomain>(instruction, in);
    }
    
    void operator()(std::shared_ptr<ltac::Jump> instruction){
        collect_jump<Reg, FloatReg, ProblemDomain>(instruction, in);
    }

    template<typename T>
    void operator()(T&){
        //Nothing to do
    }
};

template<typename Domain>
Domain meet(Domain& in, Domain& out){
    if(out.top()){
        return in;
    } else if(in.top()){
        return out;
    }

    typename Domain::Values values;
    Domain result(values);

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

} //End of anonymous namespace

ProblemDomain ltac::LiveRegistersProblem::Boundary(mtac::function_p /*function*/){
    auto value = default_element();
    return value;
}

ProblemDomain ltac::LiveRegistersProblem::Init(mtac::function_p /*function*/){
    auto value = default_element();
    return value;
}
    
PseudoProblemDomain ltac::LivePseudoRegistersProblem::Boundary(mtac::function_p /*function*/){
    auto value = default_element();
    return value;
}

PseudoProblemDomain ltac::LivePseudoRegistersProblem::Init(mtac::function_p /*function*/){
    auto value = default_element();
    return value;
}

ProblemDomain ltac::LiveRegistersProblem::meet(ProblemDomain& in, ProblemDomain& out){
    return ::meet(in, out);
}

PseudoProblemDomain ltac::LivePseudoRegistersProblem::meet(PseudoProblemDomain& in, PseudoProblemDomain& out){
    return ::meet(in, out);
}

ProblemDomain ltac::LiveRegistersProblem::transfer(mtac::basic_block_p /*basic_block*/, ltac::Statement& statement, ProblemDomain& in){
    auto out = in;
    
    LivenessCollector<ltac::Register, ltac::FloatRegister, ProblemDomain> collector(out);
    visit(collector, statement);

    return out;
}

PseudoProblemDomain ltac::LivePseudoRegistersProblem::transfer(mtac::basic_block_p /*basic_block*/, ltac::Statement& statement, PseudoProblemDomain& in){
    auto out = in;
    
    LivenessCollector<ltac::PseudoRegister, ltac::PseudoFloatRegister, PseudoProblemDomain> collector(out);
    visit(collector, statement);

    return out;
}

bool ltac::LiveRegistersProblem::optimize(ltac::Statement& /*statement*/, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> /*results*/){
    //This analysis is only made to gather information, not to optimize anything
    throw "Unimplemented";
}

bool ltac::LivePseudoRegistersProblem::optimize(ltac::Statement& /*statement*/, std::shared_ptr<mtac::DataFlowResults<PseudoProblemDomain>> /*results*/){
    //This analysis is only made to gather information, not to optimize anything
    throw "Unimplemented";
}
