//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Types.hpp"
#include "Type.hpp"
#include "assert.hpp"
#include "Platform.hpp"
#include "SymbolTable.hpp"

using namespace eddic;
    
std::shared_ptr<Type> eddic::BOOL = std::make_shared<Type>(BaseType::BOOL, false, 0, false);
std::shared_ptr<Type> eddic::INT = std::make_shared<Type>(BaseType::INT, false, 0, false);
std::shared_ptr<Type> eddic::FLOAT = std::make_shared<Type>(BaseType::FLOAT, false, 0, false);
std::shared_ptr<Type> eddic::STRING = std::make_shared<Type>(BaseType::STRING, false, 0, false);
std::shared_ptr<Type> eddic::VOID = std::make_shared<Type>(BaseType::VOID, false, 0, false);

int eddic::size(BaseType type){
    //TODO Those sizes should be stored directly inside the current platform
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

std::shared_ptr<Type> eddic::new_type(const std::string& type){
    if(type.find("[]") != std::string::npos){
        std::string baseType = type;
        baseType.resize(baseType.size() - 2);

        if(is_standard_type(baseType)){
            return new_array_type(baseType);
        } else {
            return std::make_shared<Type>(baseType, true, 0, false); 
        }
    } 

    if(is_standard_type(type)){
        return new_simple_type(type);
    } else {
        return std::make_shared<Type>(type);
    }
}

std::shared_ptr<Type> eddic::new_simple_type(const std::string& baseType, bool const_){
    return std::make_shared<Type>(stringToBaseType(baseType), false, 0, const_);
}

std::shared_ptr<Type> eddic::new_array_type(BaseType baseType, int size){
    return std::make_shared<Type>(baseType, true, size, false);
}

std::shared_ptr<Type> eddic::new_array_type(const std::string& baseType, int size){
    return std::make_shared<Type>(stringToBaseType(baseType), true, size, false);
}
