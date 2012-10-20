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
        float_registers.erase(reg);
    }
    
    void erase(PseudoFloatRegister reg){
        float_registers.erase(reg);
    }
};

std::ostream& operator<<(std::ostream& stream, LiveRegisterValues& expression);

struct LiveRegistersProblem : public DataFlowProblem<DataFlowType::Backward, LiveRegisterValues> {
    ProblemDomain Boundary(std::shared_ptr<mtac::Function> function) override;
    ProblemDomain Init(std::shared_ptr<mtac::Function> function) override;
   
    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(std::shared_ptr<mtac::BasicBlock> basic_block, mtac::Statement& statement, ProblemDomain& in) override;
    
    bool optimize(mtac::Statement& statement, std::shared_ptr<DataFlowResults<ProblemDomain>> results) override;
};

} //end of mtac

} //end of eddic

#endif
