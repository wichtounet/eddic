//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Parameter.hpp"

using namespace eddic;

Parameter::Parameter(const std::string& name, Type type, int offset) : m_name(name), m_type(type), m_offset(offset) {}

Type Parameter::type(){
    return m_type;
}

int Parameter::offset(){
    return m_offset;
}
