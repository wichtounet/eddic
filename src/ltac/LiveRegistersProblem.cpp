//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "ltac/Instruction.hpp"
#include "ltac/LiveRegistersProblem.hpp"

using namespace eddic;

typedef ltac::LiveRegistersProblem::ProblemDomain ProblemDomain;
typedef ltac::LivePseudoRegistersProblem::ProblemDomain PseudoProblemDomain;

namespace {

template<typename Reg, typename FloatReg, typename ProblemDomain>
typename std::enable_if<std::is_same<Reg, ltac::PseudoRegister>::value, void>::type collect_jump(ltac::Instruction& instruction, ProblemDomain& in){
    for(auto& reg : instruction.uses){
        in.values().insert(reg);
    }

    for(auto& reg : instruction.float_uses){
        in.values().insert(reg);
    }

    for(auto& reg : instruction.kills){
        in.values().erase(reg);
    }

    for(auto& reg : instruction.float_kills){
        in.values().erase(reg);
    }
}

template<typename Reg, typename FloatReg, typename ProblemDomain>
typename std::enable_if<std::is_same<Reg, ltac::Register>::value, void>::type collect_jump(ltac::Instruction& instruction, ProblemDomain& in){
    for(auto& reg : instruction.hard_uses){
        in.values().insert(reg);
    }

    for(auto& reg : instruction.hard_float_uses){
        in.values().insert(reg);
    }

    for(auto& reg : instruction.hard_kills){
        in.values().erase(reg);
    }

    for(auto& reg : instruction.hard_float_kills){
        in.values().erase(reg);
    }
}

template<typename Reg, typename FloatReg, typename ProblemDomain>
typename std::enable_if<std::is_same<Reg, ltac::PseudoRegister>::value, void>::type collect_instruction(ltac::Instruction& instruction, ProblemDomain& in){
    for(auto& reg : instruction.uses){
        in.values().insert(reg);
    }

    for(auto& reg : instruction.float_uses){
        in.values().insert(reg);
    }
}

template<typename Reg, typename FloatReg, typename ProblemDomain>
typename std::enable_if<std::is_same<Reg, ltac::Register>::value, void>::type collect_instruction(ltac::Instruction& instruction, ProblemDomain& in){
    for(auto& reg : instruction.hard_uses){
        in.values().insert(reg);
    }

    for(auto& reg : instruction.hard_float_uses){
        in.values().insert(reg);
    }
}

template<typename Reg, typename FloatReg, typename ProblemDomain>
struct LivenessCollector : public boost::static_visitor<> {
    ProblemDomain& in;

    LivenessCollector(ProblemDomain& in) : in(in) {}

    template<typename Arg>
    inline void set_live(Arg& arg){
        if(auto* ptr = boost::relaxed_get<Reg>(&arg)){
            in.values().insert(*ptr);
        } else if(auto* ptr = boost::relaxed_get<FloatReg>(&arg)){
            in.values().insert(*ptr);
        } else if(auto* ptr = boost::relaxed_get<ltac::Address>(&arg)){
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

    void collect(ltac::Instruction& instruction){
        if(instruction.is_jump()){
            collect_jump<Reg, FloatReg, ProblemDomain>(instruction, in);
        } else if(!instruction.is_label()) {
            if(instruction.op != ltac::Operator::NOP){
                if(ltac::erase_result_complete(instruction.op)){
                    if(auto* ptr = boost::get<ltac::Address>(&*instruction.arg1)){
                        set_live_opt(ptr->base_register);
                        set_live_opt(ptr->scaled_register);
                    } else {
                        set_dead(*instruction.arg1);
                    }
                } else {
                    set_live_opt(instruction.arg1);
                }

                set_live_opt(instruction.arg2);
                set_live_opt(instruction.arg3);
            }

            collect_instruction<Reg, FloatReg, ProblemDomain>(instruction, in);
        }
    }
};

template<typename Domain>
inline void meet(Domain& in, const Domain& out){
    if(out.top()){
        //in does not change
    } else if(in.top()){
        in = out;
    } else {
        for(auto& value : out.values().registers){
            in.values().insert(value);
        }

        for(auto& value : out.values().float_registers){
            in.values().insert(value);
        }
    }
}

} //End of anonymous namespace

ProblemDomain ltac::LiveRegistersProblem::Boundary(mtac::Function& /*function*/){
    auto value = default_element();
    return value;
}

ProblemDomain ltac::LiveRegistersProblem::Init(mtac::Function& /*function*/){
    auto value = default_element();
    return value;
}

PseudoProblemDomain ltac::LivePseudoRegistersProblem::Boundary(mtac::Function& /*function*/){
    auto value = default_element();
    return value;
}

PseudoProblemDomain ltac::LivePseudoRegistersProblem::Init(mtac::Function& /*function*/){
    auto value = default_element();
    return value;
}

void ltac::LiveRegistersProblem::meet(ProblemDomain& in, const ProblemDomain& out){
    ::meet(in, out);
}

void ltac::LivePseudoRegistersProblem::meet(PseudoProblemDomain& in, const PseudoProblemDomain& out){
    ::meet(in, out);
}

ProblemDomain ltac::LiveRegistersProblem::transfer(mtac::basic_block_p /*basic_block*/, ltac::Instruction& statement, ProblemDomain& in){
    auto out = in;

    LivenessCollector<ltac::Register, ltac::FloatRegister, ProblemDomain> collector(out);
    collector.collect(statement);

    return out;
}

PseudoProblemDomain ltac::LivePseudoRegistersProblem::transfer(mtac::basic_block_p /*basic_block*/, ltac::Instruction& statement, PseudoProblemDomain& in){
    auto out = in;

    LivenessCollector<ltac::PseudoRegister, ltac::PseudoFloatRegister, PseudoProblemDomain> collector(out);
    collector.collect(statement);

    return out;
}
