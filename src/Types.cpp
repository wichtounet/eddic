//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "Types.hpp"
#include "SemanticalException.hpp"
#include "Compiler.hpp"

using namespace eddic;

Type::Type(BaseType base, bool constant) : type(base), array(false), m_size(0), const_(constant) {}
Type::Type(BaseType base, unsigned int size, bool constant) : type(base), array(true), m_size(size), const_(constant) {}

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

int eddic::size(BaseType type){
    static int typeSizes32[(int) BaseType::COUNT] = { 8, 4, 4, 0 };
    static int typeSizes64[(int) BaseType::COUNT] = { 16, 8, 8, 0 };

    switch(platform){
        case Platform::INTEL_X86:
            return typeSizes32[(unsigned int) type];
        case Platform::INTEL_X86_64:
            return typeSizes64[(unsigned int) type];
    }
}

int eddic::size(Type type){
    if(type.isArray()){
        return size(type.base()) * type.size() + size(BaseType::INT); 
    } else {
        return size(type.base());
    }
}

bool eddic::isType(const std::string& type){
    return type == "int" || type == "void" || type == "string" || type == "bool";
}

BaseType eddic::stringToBaseType(const std::string& type){
    if (type == "int") {
        return BaseType::INT;
    } else if (type == "bool") {
        return BaseType::BOOL;
    } else if (type == "string"){
        return BaseType::STRING;
    } else if(type == "void") {
        return BaseType::VOID;
    }

    throw SemanticalException("Invalid type");
}

Type eddic::stringToType(const std::string& type){
    if (type == "int") {
        return Type(BaseType::INT, false);
    } else if (type == "bool"){
        return Type(BaseType::BOOL, false);
    } else if (type == "string"){
        return Type(BaseType::STRING, false);
    } else if(type == "int[]") {
        return Type(BaseType::INT, 0, false);       //Use a more proper way to set that it's an array type
    } else if(type == "bool[]") {
        return Type(BaseType::BOOL, 0, false);      //Use a more proper way to set that it's an array type
    } else if(type == "string[]") {
        return Type(BaseType::STRING, 0, false);    //Use a more proper way to set that it's an array type
    } else if(type == "void") {
        return Type(BaseType::VOID, false);
    }

    throw SemanticalException("Invalid type");
}
