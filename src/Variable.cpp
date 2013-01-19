//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Variable.hpp"
#include "Utils.hpp"
#include "Type.hpp"

using namespace eddic;

Variable::Variable(const std::string& name, std::shared_ptr<const Type> type, Position position) 
    : m_name(name), m_type(type), m_position(position) {}

Variable::Variable(const std::string& name, std::shared_ptr<const Type> type, Position position, Val value) 
    : m_name(name), m_type(type), m_position(position), v_value(value) {}

Variable::Variable(const std::string& name, std::shared_ptr<const Type> type, std::shared_ptr<Variable> reference, Offset offset) 
    : m_name(name), m_type(type), m_position(PositionType::TEMPORARY), m_reference(reference), m_offset(offset) {}

std::string Variable::name() const  {
    return m_name;
}

std::shared_ptr<const Type> Variable::type() const {
    return m_type;
}

Position Variable::position() const {
    return m_position;
}

Val Variable::val() const {
    return v_value;
}

void Variable::setPosition(Position position){
    m_position = position;
}
        
const ast::Position& Variable::source_position() const {
    return m_source_position;
}

void Variable::set_source_position(const ast::Position& position){
    m_source_position = position;
}
        
bool Variable::is_reference() const {
    return m_reference != nullptr;
}

std::shared_ptr<Variable> Variable::reference() const {
    return m_reference;
}

Offset Variable::reference_offset() const {
    return m_offset;
}

std::size_t eddic::Variable::references() const {
    return m_references;
}

void eddic::Variable::add_reference(){
    ++m_references;
}

std::ostream& eddic::operator<<(std::ostream& stream, const Variable& variable){
    std::string type = "";

    if(variable.type()->is_pointer()){
        type = "p";
    } else if(variable.type()->is_custom_type()){
        type = "c";
    } else if(variable.type()->is_standard_type()){
        if(variable.type() == FLOAT){
            type = "F";
        } else if(variable.type() == INT){
            type = "I";
        } else if(variable.type() == STRING){
            type = "S";
        } else if(variable.type() == CHAR){
            type = "C";
        } else if(variable.type() == BOOL){
            type = "B";
        } else {
            type = "s";
        }
    } else if(variable.type()->is_array()){
        type = "a";
    } else if(variable.type()->is_template_type()){
        type = "t";
    } else {
        type = "u";
    }

    if(variable.is_reference()){
        return stream << variable.name() << "(ref," << type << ")";
    }

    switch(variable.position().type()){
        case PositionType::STACK:
            return stream << variable.name() << "(s," << type << ")";
        case PositionType::PARAMETER:
            return stream << variable.name() << "(p," << type << ")";
        case PositionType::GLOBAL:
            return stream << variable.name() << "(g," << type << ")";
        case PositionType::CONST:
            return stream << variable.name() << "(c," << type << ")";
        case PositionType::TEMPORARY:
            return stream << variable.name() << "(t," << type << ")";
        case PositionType::VARIABLE:
            return stream << variable.name() << "(v," << type << ")";
        case PositionType::REGISTER:
            return stream << variable.name() << "(r," << type << ")";
        case PositionType::PARAM_REGISTER:
            return stream << variable.name() << "(pr," << type << ")";
    }
}

std::ostream& eddic::operator<<(std::ostream& stream, const std::shared_ptr<Variable>& variable){
    if(variable){
        return stream << *variable;
    } else {
        return stream << "null_variable";
    }
}
