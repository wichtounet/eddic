//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Type.hpp"

using namespace eddic;

Type::Type(BaseType base, bool a, unsigned int size, bool constant) : type(base), array(a), const_(constant), m_size(size) {}

BaseType Type::base() const {
    return type;
}

bool Type::isArray() const {
    return array;
}

bool Type::isConst() const {
    return const_;
}

unsigned int Type::size() const {
    return m_size;
}

bool eddic::operator==(const Type& lhs, const Type& rhs){
    return lhs.type == rhs.type && 
           lhs.array == rhs.array &&
           lhs.m_size == rhs.m_size; 
}

bool eddic::operator!=(const Type& lhs, const Type& rhs){
    return !(lhs == rhs); 
}

bool eddic::operator==(const Type& lhs, const BaseType& rhs){
    return lhs.type == rhs && lhs.array == false; 
}

bool eddic::operator!=(const Type& lhs, const BaseType& rhs){
    return !(lhs == rhs); 
}
