//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Position.hpp"

using namespace eddic;

Position::Position(PositionType type, int offset) : m_type(type), m_offset(offset), m_name("") {}
Position::Position(PositionType type, std::string name) : m_type(type), m_offset(0), m_name(name) {}

bool Position::isStack(){
    return m_type == STACK;
}

bool Position::isParameter(){
    return m_type == PARAMETER;
}

bool Position::isGlobal(){
    return m_type == GLOBAL;
}

int Position::offset(){
    return m_offset;
}

std::string Position::name(){
    return m_name;
}
