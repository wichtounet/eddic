//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "assert.hpp"
#include "Type.hpp"
#include "Types.hpp"
#include "SymbolTable.hpp"
#include "Platform.hpp"

using namespace eddic;

Type::Type(){}
Type::Type(BaseType base, bool array, unsigned int size, bool constant) : array(array), const_(constant), baseType(base), m_elements(size){}
Type::Type(const std::string& type) : custom(true), m_type(type) {}
Type::Type(const std::string& type, bool array, unsigned int size, bool const_) : array(array), const_(const_), custom(true), m_type(type), m_elements(size) {}
Type::Type(std::shared_ptr<Type> sub_type) : pointer(true), sub_type(sub_type) {}

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
            return ::size(*baseType) * elements() + INT->size(); 
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

std::shared_ptr<Type> Type::element_type() const {
    return std::make_shared<Type>(*baseType, false, 0, false);
}

std::shared_ptr<Type> Type::data_type() const {
    ASSERT(is_pointer(), "Only pointers have a data type");

    return sub_type;
}

std::shared_ptr<Type> Type::non_const() const {
    auto copy = std::make_shared<Type>();

    *copy = *this;

    copy->const_ = false;

    return copy;
}

bool eddic::operator==(std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs){
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
