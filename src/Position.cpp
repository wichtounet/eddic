//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "Position.hpp"

using namespace eddic;

Position::Position(PositionType type) : m_type(type) {}
Position::Position(PositionType type, int offset) : m_type(type), m_offset(offset) {}
Position::Position(PositionType type, const std::string& name) : m_type(type), m_name(name) {}

bool Position::isStack() const {
    return m_type == PositionType::STACK;
}

bool Position::isParameter() const {
    return m_type == PositionType::PARAMETER;
}

bool Position::isGlobal() const {
    return m_type == PositionType::GLOBAL;
}

bool Position::is_const() const {
    return m_type == PositionType::CONST;
}

bool Position::is_temporary() const {
    return m_type == PositionType::TEMPORARY;
}

bool Position::isParamRegister() const {
    return m_type == PositionType::PARAM_REGISTER;
}

bool Position::is_variable() const {
    return m_type == PositionType::VARIABLE;
}

bool Position::is_register() const {
    return m_type == PositionType::REGISTER;
}

PositionType Position::type() const {
    return m_type;
}

int Position::offset() const {
    return *m_offset;
}

const std::string& Position::name() const {
    return *m_name;
}
