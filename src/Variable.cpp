//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "Variable.hpp"
#include "Utils.hpp"
#include "Value.hpp"

#include "il/Operands.hpp"
#include "il/Operand.hpp"

using std::map;
using std::string;
using std::endl;
using std::vector;

using namespace eddic;

Variable::Variable(const std::string& name, Type type, Position position) : m_name(name), m_type(type), m_position(position) {}
Variable::Variable(const std::string& name, Type type, Position position, std::shared_ptr<Value> value) : m_name(name), m_type(type), m_position(position), m_value(value) {}
Variable::Variable(const std::string& name, Type type, Position position, Val value) : m_name(name), m_type(type), m_position(position), v_value(value) {}

std::string Variable::name() const  {
    return m_name;
}

Type Variable::type() const {
    return m_type;
}

Position Variable::position() const {
    return m_position;
}

std::shared_ptr<Value> Variable::value() const {
    return m_value;
}

Val Variable::val() const {
    return v_value;
}

std::shared_ptr<Operand> Variable::toIntegerOperand(){
    assert(m_type == Type::INT); 
    
    if(m_position.isStack()){//TODO Rename in a way that we can understand that it is a variable
        return createBaseStackOperand(-1 * m_position.offset());
    } else if(m_position.isParameter()){
        return createBaseStackOperand(m_position.offset());
    } else if(m_position.isGlobal()){
        return createGlobalOperand("VI" + m_position.name());
    }

    throw "ERROR";
}

std::pair<OperandPtr, OperandPtr> Variable::toStringOperand(){
    assert(m_type == Type::STRING); 
   
    if(m_position.isStack()){//TODO Rename in a way that we can understand that it is a variable
        return make_pair(
                    createBaseStackOperand(-1 * m_position.offset()), 
                    createBaseStackOperand(-1 * m_position.offset() - 4)
                );
    } else if(m_position.isParameter()){
        return make_pair(
                    createBaseStackOperand(m_position.offset()), 
                    createBaseStackOperand(m_position.offset() + 4)
                );
    } else if(m_position.isGlobal()){
        return make_pair(
                    createGlobalOperand("VS" + m_position.name()), 
                    createGlobalOperand("VS" + m_position.name(), 4)
                );
    }

    throw "ERROR";
}
