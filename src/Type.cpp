//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Type.hpp"

using namespace eddic;

Type::Type(BaseType base, bool array, unsigned int size, bool constant) : array(array), const_(constant), custom(false), baseType(base), m_size(size){}
Type::Type(const std::string& type) : array(false), const_(false), custom(true), m_type(type) {}

BaseType Type::base() const {
    ASSERT(is_standard_type(), "Only standard type have a base type");
    ASSERT(baseType, "The baseType has not been initialized");

    return *baseType;
}

bool Type::isArray() const {
    return array;
}

bool Type::isConst() const {
    return const_;
}

unsigned int Type::size() const {
    ASSERT(is_standard_type(), "Only standard type have a size");
    ASSERT(m_size, "The m_size has not been initialized");
    
    return *m_size;
}
        
bool Type::is_custom_type() const {
    return custom;
}

bool Type::is_standard_type() const {
    return !is_custom_type();
}

std::string Type::type() const {
    ASSERT(is_custom_type(), "Only custom type have a type");
    ASSERT(m_type, "The m_type has not been initialized");

    return *m_type;
}

bool eddic::operator==(const Type& lhs, const Type& rhs){
    return lhs.baseType == rhs.baseType && 
           lhs.array == rhs.array &&
           lhs.const_ == rhs.const_ &&
           lhs.custom == rhs.custom &&
           lhs.m_size == rhs.m_size && 
           lhs.m_type == rhs.m_type; 
}

bool eddic::operator!=(const Type& lhs, const Type& rhs){
    return !(lhs == rhs); 
}

bool eddic::operator==(const Type& lhs, const BaseType& rhs){
    return lhs.baseType == rhs && !lhs.array && !lhs.custom; 
}

bool eddic::operator!=(const Type& lhs, const BaseType& rhs){
    return !(lhs == rhs); 
}
