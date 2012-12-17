//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LIVE_REGISTERS_PROBLEM_H
#define MTAC_LIVE_REGISTERS_PROBLEM_H

#include <memory>
#include <iostream>
#include <unordered_set>

#include "assert.hpp"

#include "mtac/DataFlowProblem.hpp"

//Necessary for hash
#include "ltac/Register.hpp"
#include "ltac/FloatRegister.hpp"
#include "ltac/PseudoRegister.hpp"
#include "ltac/PseudoFloatRegister.hpp"

namespace eddic {

namespace ltac {

template<typename Reg, typename FloatReg>
struct LiveRegisterValues {
    std::unordered_set<Reg> registers;
    std::unordered_set<FloatReg> float_registers;

    void insert(const Reg& reg){
        registers.insert(reg);
    }

    void insert(const FloatReg& reg){
        float_registers.insert(reg);
    }

    auto find(const Reg& reg) -> decltype(registers.find(reg)) {
        return registers.find(reg);
    }
    
    auto find(const FloatReg& reg) -> decltype(float_registers.find(reg)) {
        return float_registers.find(reg);
    }
    
    auto end() -> decltype(registers.end()) {
        return registers.end();
    }
    
    auto fend() -> decltype(float_registers.end()) {
        return float_registers.end();
    }
    
    void erase(const Reg& reg){
        registers.erase(reg);
    }

    void erase(const FloatReg& reg){
        float_registers.erase(reg);
    }

    std::size_t size(){
        return (static_cast<std::size_t>(std::numeric_limits<unsigned short>::max()) + 1) * registers.size() + float_registers.size();
    }
};

//Liveness analysis on Hard Registers

struct LiveRegistersProblem : public mtac::DataFlowProblem<mtac::DataFlowType::Low_Backward, LiveRegisterValues<ltac::Register, ltac::FloatRegister>> {
    ProblemDomain Boundary(mtac::function_p function) override;
    ProblemDomain Init(mtac::function_p function) override;
   
    void meet(ProblemDomain& in, const ProblemDomain& out) override;

    ProblemDomain transfer(mtac::basic_block_p basic_block, ltac::Statement& statement, ProblemDomain& in) override;
    ProblemDomain transfer(mtac::basic_block_p, mtac::Statement&, ProblemDomain&) override { eddic_unreachable("Not MTAC"); };
    
    bool optimize(ltac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> results) override;
    bool optimize(mtac::Statement&, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> ) override { eddic_unreachable("Not MTAC"); };
};

//Liveness analysis on Pseudo Registers

struct LivePseudoRegistersProblem : public mtac::DataFlowProblem<mtac::DataFlowType::Low_Backward, LiveRegisterValues<ltac::PseudoRegister, ltac::PseudoFloatRegister>> {
    ProblemDomain Boundary(mtac::function_p function) override;
    ProblemDomain Init(mtac::function_p function) override;
   
    void meet(ProblemDomain& in, const ProblemDomain& out) override;

    ProblemDomain transfer(mtac::basic_block_p basic_block, ltac::Statement& statement, ProblemDomain& in) override;
    ProblemDomain transfer(mtac::basic_block_p, mtac::Statement&, ProblemDomain&) override { eddic_unreachable("Not MTAC"); };
    
    bool optimize(ltac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> results) override;
    bool optimize(mtac::Statement&, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> ) override { eddic_unreachable("Not MTAC"); };
};

template<typename Reg, typename FloatReg>
std::ostream& operator<<(std::ostream& stream, const LiveRegisterValues<Reg, FloatReg>& value){
    stream << "set{";

    for(auto& v : value.registers){
        stream << v << ", ";
    }
    
    for(auto& v : value.float_registers){
        stream << v << ", ";
    }

    return stream << "}";
}

} //end of mtac

} //end of eddic

#endif
