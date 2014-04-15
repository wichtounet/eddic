//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef MTAC_LIVE_REGISTERS_PROBLEM_H
#define MTAC_LIVE_REGISTERS_PROBLEM_H

#include <memory>
#include <ostream>
#include <unordered_set>

#include <boost/utility.hpp>

#define STATIC_CONSTANT(type,name,value) BOOST_STATIC_CONSTANT(type, name = value)

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

struct LiveRegistersProblem {
    //The type of data managed
    typedef mtac::Domain<LiveRegisterValues<ltac::Register, ltac::FloatRegister>> ProblemDomain;

    //The direction
    STATIC_CONSTANT(mtac::DataFlowType, Type, mtac::DataFlowType::Backward);
    STATIC_CONSTANT(bool, Low, true);

    ProblemDomain Boundary(mtac::Function& function);
    ProblemDomain Init(mtac::Function& function);
   
    void meet(ProblemDomain& in, const ProblemDomain& out);
    ProblemDomain transfer(mtac::basic_block_p basic_block, ltac::Instruction& statement, ProblemDomain& in);

    ProblemDomain top_element(){
        return ProblemDomain();
    }

    ProblemDomain default_element(){
        return ProblemDomain(ProblemDomain::Values());
    }
};

//Liveness analysis on Pseudo Registers

struct LivePseudoRegistersProblem {
    //The type of data managed
    typedef mtac::Domain<LiveRegisterValues<ltac::PseudoRegister, ltac::PseudoFloatRegister>> ProblemDomain;

    //The direction
    STATIC_CONSTANT(mtac::DataFlowType, Type, mtac::DataFlowType::Backward);
    STATIC_CONSTANT(bool, Low, true);

    ProblemDomain Boundary(mtac::Function& function);
    ProblemDomain Init(mtac::Function& function);
   
    void meet(ProblemDomain& in, const ProblemDomain& out);
    ProblemDomain transfer(mtac::basic_block_p basic_block, ltac::Instruction& statement, ProblemDomain& in);

    ProblemDomain top_element(){
        return ProblemDomain();
    }

    ProblemDomain default_element(){
        return ProblemDomain(ProblemDomain::Values());
    }
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
