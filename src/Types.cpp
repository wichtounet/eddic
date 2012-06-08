//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Types.hpp"
#include "Platform.hpp"
#include "SymbolTable.hpp"

using namespace eddic;
    
Type eddic::BOOL(BaseType::BOOL, false, 0, false);
Type eddic::INT(BaseType::INT, false, 0, false);
Type eddic::FLOAT(BaseType::FLOAT, false, 0, false);

int eddic::size(BaseType type){
    static int typeSizes32[BASETYPE_COUNT] = {  8, 4, 4, 4, 0 };
    static int typeSizes64[BASETYPE_COUNT] = { 16, 8, 8, 8, 0 };

    switch(platform){
        case Platform::INTEL_X86:
            return typeSizes32[(unsigned int) type];
        case Platform::INTEL_X86_64:
            return typeSizes64[(unsigned int) type];
    }

    return -1;
}

int eddic::size(Type type){
    if(type.is_standard_type()){
        if(type.isArray()){
            return size(type.base()) * type.size() + size(BaseType::INT); 
        } else {
            return size(type.base());
        }
    } else {
        ASSERT(!type.isArray(), "Array of custom types are not supported for now");

        return symbols.size_of_struct(type.type());
    }
}

bool eddic::is_standard_type(const std::string& type){
    return type == "int" || type == "void" || type == "string" || type == "bool" || type == "float";
}

BaseType stringToBaseType(const std::string& type){
    ASSERT(is_standard_type(type), "The given type is not standard");

    if (type == "int") {
        return BaseType::INT;
    } else if (type == "bool") {
        return BaseType::BOOL;
    } else if (type == "float"){
        return BaseType::FLOAT;
    } else if (type == "string"){
        return BaseType::STRING;
    }
    
    return BaseType::VOID;
}

Type eddic::new_type(const std::string& type){
    if(type.find("[]") != std::string::npos){
        std::string baseType = type;
        baseType.resize(baseType.size() - 2);

        if(is_standard_type(baseType)){
            return newArrayType(baseType);
        } else {
            return Type(baseType, true, 0, false); 
        }
    } 

    if(is_standard_type(type)){
        return newSimpleType(type);
    } else {
        return Type(type);
    }
}

Type eddic::newSimpleType(BaseType baseType, bool const_){
    return Type(baseType, false, 0, const_);
}

Type eddic::newSimpleType(const std::string& baseType, bool const_){
    return Type(stringToBaseType(baseType), false, 0, const_);
}

Type eddic::newArrayType(BaseType baseType, int size){
    return Type(baseType, true, size, false);
}

Type eddic::newArrayType(const std::string& baseType, int size){
    return Type(stringToBaseType(baseType), true, size, false);
}
