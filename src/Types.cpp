//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Types.hpp"
#include "Type.hpp"
#include "assert.hpp"
#include "SymbolTable.hpp"

using namespace eddic;
    
std::shared_ptr<Type> eddic::BOOL = std::make_shared<Type>(BaseType::BOOL, false);
std::shared_ptr<Type> eddic::INT = std::make_shared<Type>(BaseType::INT, false);
std::shared_ptr<Type> eddic::FLOAT = std::make_shared<Type>(BaseType::FLOAT, false);
std::shared_ptr<Type> eddic::STRING = std::make_shared<Type>(BaseType::STRING, false);
std::shared_ptr<Type> eddic::VOID = std::make_shared<Type>(BaseType::VOID, false);

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

std::shared_ptr<Type> eddic::new_type(const std::string& type, bool const_){
    if(type.find("[]") != std::string::npos){
        std::string baseType = type;
        baseType.resize(baseType.size() - 2);

        return new_array_type(new_type(baseType));
    } 

    if(is_standard_type(type)){
        return std::make_shared<Type>(stringToBaseType(type), const_);
    } else {
        assert(!const_);
        return std::make_shared<Type>(type);
    }
}

std::shared_ptr<Type> eddic::new_array_type(std::shared_ptr<Type> data_type, int size){
    return std::make_shared<Type>(data_type, size);
}
