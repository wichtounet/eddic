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

std::shared_ptr<const Type> eddic::BOOL = std::make_shared<StandardType>(BaseType::BOOL, false);
std::shared_ptr<const Type> eddic::INT = std::make_shared<StandardType>(BaseType::INT, false);
std::shared_ptr<const Type> eddic::FLOAT = std::make_shared<StandardType>(BaseType::FLOAT, false);
std::shared_ptr<const Type> eddic::STRING = std::make_shared<StandardType>(BaseType::STRING, false);
std::shared_ptr<const Type> eddic::VOID = std::make_shared<StandardType>(BaseType::VOID, false);

/* Implementation of Type */ 

Type::Type() {}

bool Type::is_array() const {
    return false;
}

bool Type::is_pointer() const {
    return false;
}
        
bool Type::is_custom_type() const {
    return false;
}

bool Type::is_standard_type() const {
    return false;
}

bool Type::is_const() const {
    return false;
}

unsigned int Type::size() const {
    ASSERT_PATH_NOT_TAKEN("Not specialized type");
}

unsigned int Type::elements() const {
    ASSERT_PATH_NOT_TAKEN("Not an array type");
}

std::string Type::type() const {
    ASSERT_PATH_NOT_TAKEN("Not a custom type");
}

std::shared_ptr<const Type> Type::data_type() const {
    ASSERT_PATH_NOT_TAKEN("No data type");
}

BaseType Type::base() const {
    ASSERT_PATH_NOT_TAKEN("Not a standard type");
}

std::shared_ptr<const Type> Type::non_const() const {
    if(is_const()){
        return shared_from_this();
    }

    if(is_array()){
        return shared_from_this();
    } 
    
    if(is_custom_type()){
        return shared_from_this();
    } 
    
    if(is_standard_type()){
        return std::make_shared<StandardType>(base(), false);
    } 

    assert(is_pointer());
    
    return shared_from_this();
}

bool eddic::operator==(std::shared_ptr<const Type> lhs, std::shared_ptr<const Type> rhs){
    if(lhs->is_array()){
        return rhs->is_array() && lhs->data_type() == rhs->data_type() && lhs->elements() == rhs->elements();
    }

    if(lhs->is_pointer()){
        return rhs->is_pointer() && lhs->data_type() == rhs->data_type();
    }

    if(lhs->is_custom_type()){
        return rhs->is_custom_type() && lhs->type() == rhs->type();
    }

    if(lhs->is_standard_type()){
        return rhs->is_standard_type() && lhs->base() == rhs->base();
    }

    return false;
}

bool eddic::operator!=(std::shared_ptr<const Type> lhs, std::shared_ptr<const Type> rhs){
    return !(lhs == rhs); 
}

/* Implementation of StandardType  */

StandardType::StandardType(BaseType type, bool const_) : base_type(type), const_(const_) {}

BaseType StandardType::base() const {
    return base_type;
}

bool StandardType::is_standard_type() const {
    return true;
}

bool StandardType::is_const() const {
    return const_;
}

unsigned int StandardType::size() const {
    auto descriptor = getPlatformDescriptor(platform);
    return descriptor->size_of(base());
}

/* Implementation of CustomType */

CustomType::CustomType(const std::string& type) : m_type(type) {}

std::string CustomType::type() const {
    return m_type;
}

bool CustomType::is_custom_type() const {
    return true;
}

unsigned int CustomType::size() const {
    return symbols.size_of_struct(type());
}
        
/* Implementation of ArrayType  */

ArrayType::ArrayType(std::shared_ptr<const Type> sub_type, int size) : sub_type(sub_type), m_elements(size) {}

unsigned int ArrayType::elements() const {
    return m_elements;
}

std::shared_ptr<const Type> ArrayType::data_type() const {
    return sub_type;
}

bool ArrayType::is_array() const {
    return true;
}

unsigned int ArrayType::size() const {
    return data_type()->size() * elements() + INT->size(); 
}
        
/* Implementation of PointerType  */

PointerType::PointerType(std::shared_ptr<const Type> sub_type) : sub_type(sub_type) {} 

std::shared_ptr<const Type> PointerType::data_type() const {
    return sub_type;
}

bool PointerType::is_pointer() const {
    return true;
}

unsigned int PointerType::size() const {
    return INT->size();
}

/* Implementation of factories  */

//TODO Avoid instantiate a type for these values
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

std::shared_ptr<const Type> eddic::new_type(const std::string& type, bool const_){
    //Parse array types
    if(type.find("[]") != std::string::npos){
        std::string baseType = type;
        baseType.resize(baseType.size() - 2);

        return new_array_type(new_type(baseType));
    }
    
    //Parse pointer types
    if(type.find("*") != std::string::npos){
        std::string baseType = type;
        baseType.resize(baseType.size() - 1);

        return new_pointer_type(new_type(baseType));
    }

    if(is_standard_type(type)){
        return std::make_shared<StandardType>(stringToBaseType(type), const_);
    } else {
        assert(!const_);
        return std::make_shared<CustomType>(type);
    }
}

std::shared_ptr<const Type> eddic::new_array_type(std::shared_ptr<const Type> data_type, int size){
    return std::make_shared<ArrayType>(data_type, size);
}

std::shared_ptr<const Type> eddic::new_pointer_type(std::shared_ptr<const Type> data_type){
    return std::make_shared<PointerType>(data_type);
}

bool eddic::is_standard_type(const std::string& type){
    return type == "int" || type == "void" || type == "string" || type == "bool" || type == "float";
}
