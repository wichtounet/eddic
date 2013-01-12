//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <atomic>

#include "assert.hpp"
#include "Function.hpp"

#include "mtac/Quadruple.hpp"

using namespace eddic;

static std::atomic<std::size_t> uid_counter(0);

mtac::Quadruple::Quadruple() : _uid(++uid_counter) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(mtac::Operator o) : _uid(++uid_counter), op(o) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(std::shared_ptr<Variable> result, mtac::Argument a1, mtac::Operator o) : _uid(++uid_counter), result(result), arg1(a1), op(o) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(std::shared_ptr<Variable> result, mtac::Argument a1, mtac::Operator o, mtac::Argument a2) : _uid(++uid_counter), result(result), arg1(a1), arg2(a2), op(o) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(mtac::Operator o, mtac::Argument a1) : _uid(++uid_counter), arg1(a1), op(o) {
    //Nothing to init    
}

mtac::Quadruple::Quadruple(mtac::Operator o, mtac::Argument a1, mtac::Argument a2) : _uid(++uid_counter), arg1(a1), arg2(a2), op(o) {
    //Nothing to init    
}
    
mtac::Quadruple::Quadruple(const std::string& param, mtac::Operator op) : _uid(++uid_counter), op(op), m_param(param){
    //Nothing to init
}

mtac::Quadruple::Quadruple(mtac::Operator op, mtac::Argument arg, std::shared_ptr<Variable> param, eddic::Function& function) : _uid(++uid_counter), result(param), arg1(arg), op(op), m_function(&function) {
    //Nothing to init
}

mtac::Quadruple::Quadruple(mtac::Operator op, mtac::Argument arg, const std::string& param, eddic::Function& function) : _uid(++uid_counter), arg1(arg), op(op), m_function(&function), m_param(param){
    //Nothing to init
}

mtac::Quadruple::Quadruple(mtac::Operator op, eddic::Function& function, std::shared_ptr<Variable> return1, std::shared_ptr<Variable> return2) : _uid(++uid_counter), result(return1), op(op), secondary(return2), m_function(&function){
    eddic_assert(m_function, "Function is mandatory for calls");
}

mtac::Quadruple::Quadruple(mtac::Operator op, mtac::Argument arg, const std::string& label) : _uid(++uid_counter), arg1(arg), op(op), m_param(label) {
    //Nothing to init
}

mtac::Quadruple::Quadruple(const mtac::Quadruple& rhs) : 
        _uid(++uid_counter), 
        result(rhs.result), 
        arg1(rhs.arg1),
        arg2(rhs.arg2),
        op(rhs.op),
        size(rhs.size),
        address(rhs.address),
        secondary(rhs.secondary),
        m_function(rhs.m_function),
        m_param(rhs.m_param),
        block(rhs.block)
{
    //There is a new reference to the called function
    if(op == mtac::Operator::CALL){
        ++m_function->references();
    }
}

mtac::Quadruple& mtac::Quadruple::operator=(const mtac::Quadruple& rhs){
    //No need to assign this into this
    if(this == &rhs){
        return *this;
    }
    
    _uid = ++uid_counter;
    result = rhs.result;
    arg1 = rhs.arg1;
    arg2 = rhs.arg2;
    op = rhs.op;
    size = rhs.size;
    block = rhs.block;
    address = rhs.address;
    m_function = rhs.m_function;
    m_param = rhs.m_param;
    secondary = rhs.secondary;

    //There is a new reference to the called function
    if(op == mtac::Operator::CALL){
        ++m_function->references();
    }

    return *this;
}

const std::string& mtac::Quadruple::label() const {
    return m_param;
}

const std::string& mtac::Quadruple::std_param() const {
    return m_param;
}

std::shared_ptr<Variable> mtac::Quadruple::param(){
    return result;
}

eddic::Function& mtac::Quadruple::function(){
    eddic_assert(m_function, "function() can only be called on operations that support it");

    return *m_function;
}

std::shared_ptr<Variable> mtac::Quadruple::return1(){
    return result;
}

std::shared_ptr<Variable> mtac::Quadruple::return2(){
    return secondary;
}

bool mtac::Quadruple::is_if(){
    return op >= mtac::Operator::IF_UNARY && op <= mtac::Operator::IF_FL;
}

bool mtac::Quadruple::is_if_false(){
    return op >= mtac::Operator::IF_FALSE_UNARY && op <= mtac::Operator::IF_FALSE_FL;
}

bool mtac::Quadruple::operator==(const mtac::Quadruple& rhs){
    return _uid == rhs._uid;
}

bool mtac::Quadruple::operator!=(const mtac::Quadruple& rhs){
    return !(*this == rhs);
}
