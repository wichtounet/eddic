//=======================================================================
// Copyright Baptiste Wicht 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <atomic>

#include "ltac/Instruction.hpp"

using namespace eddic;

static std::atomic<std::size_t> uid_counter(0);
    
//Default constructor
ltac::Instruction::Instruction() : _uid(++uid_counter) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op) : _uid(++uid_counter), op(op) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1) : _uid(++uid_counter), op(op), arg1(arg1) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2) : _uid(++uid_counter), op(op), arg1(arg1), arg2(arg2) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2, ltac::Argument arg3) : _uid(++uid_counter), op(op), arg1(arg1), arg2(arg2), arg3(arg3) {
    //Nothing to init
}
        
bool ltac::Instruction::is_jump() const {
    return op >= ltac::Operator::ALWAYS && op <= ltac::Operator::NZ;
}
