//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Position.hpp"

using namespace eddic;

Position::Position(PositionType type) : m_type(type), m_offset(0), m_name("") {}
Position::Position(PositionType type, int offset) : m_type(type), m_offset(offset), m_name("") {}
Position::Position(PositionType type, const std::string& name) : m_type(type), m_offset(0), m_name(name) {}

bool Position::isStack() const {
    return m_type == STACK;
}

bool Position::isParameter() const {
    return m_type == PARAMETER;
}

bool Position::isGlobal() const {
    return m_type == GLOBAL;
}

bool Position::isConst() const {
    return m_type == CONST;
}

int Position::offset() const {
    return m_offset;
}

PositionType Position::type() const {
    return m_type;
}

const std::string& Position::name() const {
    return m_name;
}
