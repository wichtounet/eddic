//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
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

struct PseudoRegister;
struct PseudoFloatRegister;
    
typedef std::unordered_set<PseudoRegister> PseudoRegisters;
typedef std::unordered_set<PseudoFloatRegister> PseudoFloatRegisters;

struct LiveRegisterValues {
    PseudoRegisters registers;
    PseudoFloatRegisters float_registers;

    void insert(PseudoRegister reg);
    void insert(PseudoFloatRegister reg);
    void erase(PseudoRegister reg);
    void erase(PseudoFloatRegister reg);
    std::size_t size();
};

std::ostream& operator<<(std::ostream& stream, LiveRegisterValues& expression);

struct LiveRegistersProblem : public mtac::DataFlowProblem<mtac::DataFlowType::Low_Backward, LiveRegisterValues> {
    ProblemDomain Boundary(mtac::function_p function) override;
    ProblemDomain Init(mtac::function_p function) override;
   
    ProblemDomain meet(ProblemDomain& in, ProblemDomain& out) override;
    ProblemDomain transfer(mtac::basic_block_p basic_block, ltac::Statement& statement, ProblemDomain& in) override;
    ProblemDomain transfer(mtac::basic_block_p, mtac::Statement&, ProblemDomain&) override { ASSERT_PATH_NOT_TAKEN("Not MTAC"); };
    
    bool optimize(ltac::Statement& statement, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> results) override;
    bool optimize(mtac::Statement&, std::shared_ptr<mtac::DataFlowResults<ProblemDomain>> ) override { ASSERT_PATH_NOT_TAKEN("Not MTAC"); };
};

} //end of mtac

} //end of eddic

#endif
