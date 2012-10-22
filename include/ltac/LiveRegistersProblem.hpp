//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef MTAC_LIVE_REGISTERS_PROBLEM_H
#define MTAC_LIVE_REGISTERS_PROBLEM_H

#include <unordered_set>
#include <memory>
#include <iostream>

#include "assert.hpp"

#include "mtac/DataFlowProblem.hpp"

namespace eddic {

namespace ltac {
    
typedef std::unordered_set<PseudoRegister> PseudoRegisters;
typedef std::unordered_set<PseudoFloatRegister> PseudoFloatRegisters;

struct LiveRegisterValues {
    PseudoRegisters registers;
    PseudoFloatRegisters float_registers;

    void insert(PseudoRegister reg){
        registers.insert(reg);
    }
    
    void insert(PseudoFloatRegister reg){
        float_registers.insert(reg);
    }
    
    void erase(PseudoRegister reg){
        registers.erase(reg);
    }
    
    void erase(PseudoFloatRegister reg){
        float_registers.erase(reg);
    }

    std::size_t size(){
       return (static_cast<std::size_t>(std::numeric_limits<unsigned short>::max()) + 1) * registers.size() + float_registers.size();
    }
};

std::ostream& operator<<(std::ostream& stream, LiveRegisterValues& expression);

struct LiveRegistersProblem : public mtac::DataFlowProblem<mtac::DataFlowType::Low_Backward, LiveRegisterValues> {
    ProblemDomain Boundary(std::shared_ptr<mtac::Function> function) override;
    ProblemDomain Init(std::shared_ptr<mtac::Function> function) override;
   
    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, ltac::Statement& statement, ProblemDomain& in) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock>, mtac::Statement&, ProblemDomain&) override { ASSERT_PATH_NOT_TAKEN("Not MTAC"); };
    
    bool optimize(ltac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> results) override;
    bool optimize(mtac::Statement&, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> ) override { ASSERT_PATH_NOT_TAKEN("Not MTAC"); };
};

} //end of mtac

} //end of eddic

#endif
