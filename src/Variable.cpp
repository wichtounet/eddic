//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"
#include "Utils.hpp"
#include "Type.hpp"

using namespace eddic;

Variable::Variable(const std::string& name, std::shared_ptr<Type> type, Position position) : m_name(name), m_type(type), m_position(position) {}
Variable::Variable(const std::string& name, std::shared_ptr<Type> type, Position position, Val value) : m_name(name), m_type(type), m_position(position), v_value(value) {}

std::string Variable::name() const  {
    return m_name;
}

std::shared_ptr<Type> Variable::type() const {
    return m_type;
}

Position Variable::position() const {
    return m_position;
}

Val Variable::val() const {
    return v_value;
}

void Variable::addReference(){
    ++references;
}

int Variable::referenceCount() const {
    return references;
}

void Variable::setPosition(Position position){
    m_position = position;
}
