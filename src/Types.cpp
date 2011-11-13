//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "Types.hpp"
#include "SemanticalException.hpp"

using namespace eddic;

Type::Type(BaseType base) : type(base), array(false), m_size(0) {}
Type::Type(BaseType base, unsigned int size) : type(base), array(true), m_size(size)

BaseType Type::base() const {
    return type;
}

bool Type::isArray() const {
    return array;
}

unsigned int Type::size() const {
    return m_size;
}

const int typeSizes[(int) Type::COUNT] = { 8, 4, 0 };

int eddic::size(BaseType type){
    return typeSizes[(unsigned int) type];
}

int eddic::size(Type type){
    if(type.isArray()){
        return size(type.base()) * type.size() + size(BaseType::INT); 
    } else {
        return size(type.base());
    }
}

bool eddic::isType(const std::string& type){
    return type == "int" || type == "void" || type == "string";
}

Type eddic::stringToType(const std::string& type){
    if (type == "int") {
        return BaseType(Type::INT);
    } else if (type == "string"){
        return BaseType(Type::STRING);
    } else if(type == "void") {
        return BaseType(Type::VOID);
    }

    throw SemanticalException("Invalid type");
}
