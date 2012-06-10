//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Type.hpp"
#include "SymbolTable.hpp"
#include "Platform.hpp"

using namespace eddic;

Type::Type(BaseType base, bool constant) : const_(constant), baseType(base) {}
Type::Type(const std::string& type) : custom(true), m_type(type) {}
Type::Type(std::shared_ptr<Type> sub_type, int size) : array(true), sub_type(sub_type), m_elements(size) {}

bool Type::is_array() const {
    return array;
}

bool Type::is_const() const {
    return const_;
}

bool Type::is_pointer() const {
    return pointer;
}

unsigned int Type::elements() const {
    ASSERT(is_standard_type(), "Only standard type have a size");
    ASSERT(m_elements, "The m_elements has not been initialized");
    
    return *m_elements;
}
        
bool Type::is_custom_type() const {
    return custom;
}

bool Type::is_standard_type() const {
    return !is_custom_type();
}

int size(BaseType type){
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

unsigned int Type::size() const {
    if(is_standard_type()){
        if(is_array()){
            return sub_type->size() * elements() + INT->size(); 
        } else {
            return ::size(*baseType);
        }
    } else {
        return symbols.size_of_struct(type());
    }
}

std::string Type::type() const {
    ASSERT(is_custom_type(), "Only custom type have a type");
    ASSERT(m_type, "The m_type has not been initialized");

    return *m_type;
}

std::shared_ptr<Type> Type::data_type() const {
    ASSERT(is_pointer() || is_array(), "Only pointers have a data type");

    return sub_type;
}

std::shared_ptr<Type> Type::non_const() const {
    if(is_array()){
        return std::make_shared<Type>(sub_type);
    } else if(is_custom_type()){
        return std::make_shared<Type>(type());
    } else if(is_standard_type()){
        return std::make_shared<Type>(*baseType, false);
    }

    ASSERT_PATH_NOT_TAKEN("Unhandled conversion");
}

bool eddic::operator==(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs){
    if(lhs->array){
        return rhs->array && lhs->sub_type == rhs->sub_type && lhs->m_elements == rhs->m_elements;
    }

    return lhs->baseType == rhs->baseType && 
           lhs->array == rhs->array &&
           lhs->const_ == rhs->const_ &&
           lhs->custom == rhs->custom &&
           lhs->m_elements == rhs->m_elements && 
           lhs->m_type == rhs->m_type; 
}

bool eddic::operator!=(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs){
    return !(lhs == rhs); 
}

bool eddic::operator==(std::shared_ptr<Type> lhs, const BaseType& rhs){
    return lhs->baseType == rhs && !lhs->array && !lhs->custom; 
}

bool eddic::operator!=(std::shared_ptr<Type> lhs, const BaseType& rhs){
    return !(lhs == rhs); 
}

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
