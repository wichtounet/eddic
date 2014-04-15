//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <atomic>

#include "ltac/Instruction.hpp"

using namespace eddic;

static std::atomic<std::size_t> uid_counter(0);

ltac::Instruction::Instruction(ltac::Operator op, tac::Size size) : 
        _uid(++uid_counter), op(op), size(size) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1, tac::Size size) : 
        _uid(++uid_counter), op(op), arg1(arg1), size(size) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2, tac::Size size) : 
        _uid(++uid_counter), op(op), arg1(arg1), arg2(arg2), size(size) {
    //Nothing to init
}

ltac::Instruction::Instruction(ltac::Operator op, ltac::Argument arg1, ltac::Argument arg2, ltac::Argument arg3, tac::Size size) : 
        _uid(++uid_counter), op(op), arg1(arg1), arg2(arg2), arg3(arg3), size(size) {
    //Nothing to init
}

ltac::Instruction::Instruction(std::string label, ltac::Operator op, tac::Size size) : 
        _uid(++uid_counter), op(op), label(std::move(label)), size(size) {
    //Nothing to init
}

ltac::Instruction::Instruction(const ltac::Instruction& rhs) :  
    _uid(++uid_counter), 
    op(rhs.op),
    arg1(rhs.arg1),
    arg2(rhs.arg2),
    arg3(rhs.arg3),
    size(rhs.size),
    label(rhs.label),
    target_function(rhs.target_function),
    uses(rhs.uses),
    float_uses(rhs.float_uses),
    hard_uses(rhs.hard_uses),
    hard_float_uses(rhs.hard_float_uses),
    kills(rhs.kills),
    float_kills(rhs.float_kills),
    hard_kills(rhs.hard_kills),
    hard_float_kills(rhs.hard_float_kills)
{
    //Nothing to init
}

ltac::Instruction& ltac::Instruction::operator=(const ltac::Instruction& rhs){
    //No need to assign this into this
    if(this == &rhs){
        return *this;
    }
    
    _uid = ++uid_counter; 
    op = rhs.op;
    arg1 = rhs.arg1;
    arg2 = rhs.arg2;
    arg3 = rhs.arg3;
    size = rhs.size;
    label = rhs.label;
    target_function = rhs.target_function;
    uses = rhs.uses;
    float_uses = rhs.float_uses;
    hard_uses = rhs.hard_uses;
    hard_float_uses = rhs.hard_float_uses;
    kills = rhs.kills;
    float_kills = rhs.float_kills;
    hard_kills = rhs.hard_kills;
    hard_float_kills = rhs.hard_float_kills;

    return *this;
}

ltac::Instruction::Instruction(ltac::Instruction&& rhs) noexcept :
    _uid(std::move(rhs._uid)), 
    op(std::move(rhs.op)),
    arg1(std::move(rhs.arg1)),
    arg2(std::move(rhs.arg2)),
    arg3(std::move(rhs.arg3)),
    size(std::move(rhs.size)),
    label(std::move(rhs.label)),
    target_function(std::move(rhs.target_function)),
    uses(std::move(rhs.uses)),
    float_uses(std::move(rhs.float_uses)),
    hard_uses(std::move(rhs.hard_uses)),
    hard_float_uses(std::move(rhs.hard_float_uses)),
    kills(std::move(rhs.kills)),
    float_kills(std::move(rhs.float_kills)),
    hard_kills(std::move(rhs.hard_kills)),
    hard_float_kills(std::move(rhs.hard_float_kills))
{
    rhs._uid = 0;
}

ltac::Instruction& ltac::Instruction::operator=(ltac::Instruction&& rhs) noexcept {
    //No need to assign this into this
    if(this == &rhs){
        return *this;
    }
    
    _uid = std::move(rhs._uid);
    op = std::move(rhs.op);
    arg1 = std::move(rhs.arg1);
    arg2 = std::move(rhs.arg2);
    arg3 = std::move(rhs.arg3);
    size = std::move(rhs.size);
    label = std::move(rhs.label);
    target_function = std::move(rhs.target_function);
    uses = std::move(rhs.uses);
    float_uses = std::move(rhs.float_uses);
    hard_uses = std::move(rhs.hard_uses);
    hard_float_uses = std::move(rhs.hard_float_uses);
    kills = std::move(rhs.kills);
    float_kills = std::move(rhs.float_kills);
    hard_kills = std::move(rhs.hard_kills);
    hard_float_kills = std::move(rhs.hard_float_kills);

    rhs._uid = 0;

    return *this;
}
        
bool ltac::Instruction::is_jump() const {
    return op >= ltac::Operator::ALWAYS && op <= ltac::Operator::NZ;
}

bool ltac::Instruction::is_label() const {
    return op == ltac::Operator::LABEL;
}
