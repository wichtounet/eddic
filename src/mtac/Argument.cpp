//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include "Utils.hpp"
#include "Variable.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"

#include "mtac/Argument.hpp"

using namespace eddic;

bool eddic::mtac::operator==(const  mtac::Argument& a, const mtac::Argument& b){ 
    return eddi_detail::variant_equals()(a, b); 
}

bool eddic::mtac::operator==(const mtac::Argument& a, int b){
    return boost::get<int>(&a) && boost::get<int>(a) == b;
}

bool eddic::mtac::operator==(const mtac::Argument& a, double b){
    return boost::get<double>(&a) && boost::get<double>(a) == b;
}

bool eddic::mtac::operator==(const mtac::Argument& a, const std::string& b){
    return boost::get<std::string>(&a) && boost::get<std::string>(a) == b;
}

bool eddic::mtac::operator==(const mtac::Argument& a, std::shared_ptr<Variable> b){
    return boost::get<std::shared_ptr<Variable>>(&a) && boost::get<std::shared_ptr<Variable>>(a) == b;
}

namespace {

struct ArgumentToString : public boost::static_visitor<std::ostream&> {
   std::ostream& stream;

   ArgumentToString(std::ostream& stream) : stream(stream) {}

   std::ostream& operator()(const std::shared_ptr<Variable>& variable) const {
        std::string type = "";

        if(variable->type()->is_pointer()){
            type = "p";
        } else if(variable->type()->is_custom_type()){
            type = "c";
        } else if(variable->type()->is_standard_type()){
            if(variable->type() == FLOAT){
                type = "F";
            } else if(variable->type() == INT){
                type = "I";
            } else if(variable->type() == STRING){
                type = "S";
            } else if(variable->type() == CHAR){
                type = "C";
            } else if(variable->type() == BOOL){
                type = "B";
            } else {
                type = "s";
            }
        } else if(variable->type()->is_array()){
            type = "a";
        } else if(variable->type()->is_template_type()){
            type = "t";
        } else {
            type = "u";
        }

        if(variable->is_reference()){
            return stream << variable->name() << "(ref," << type << ")";
        }

        switch(variable->position().type()){
            case PositionType::STACK:
                return stream << variable->name() << "(s," << type << ")";
            case PositionType::PARAMETER:
                return stream << variable->name() << "(p," << type << ")";
            case PositionType::GLOBAL:
                return stream << variable->name() << "(g," << type << ")";
            case PositionType::CONST:
                return stream << variable->name() << "(c," << type << ")";
            case PositionType::TEMPORARY:
                return stream << variable->name() << "(t," << type << ")";
            case PositionType::VARIABLE:
                return stream << variable->name() << "(v," << type << ")";
            case PositionType::REGISTER:
                return stream << variable->name() << "(r," << type << ")";
            case PositionType::PARAM_REGISTER:
                return stream << variable->name() << "(pr," << type << ")";
        }
   }

   std::ostream& operator()(const int& integer) const {
        return stream << toString(integer);
   }
   
   std::ostream& operator()(const double& float_) const {
        return stream << toString(float_);
   }

   std::ostream& operator()(const std::string& str) const {
        return stream << str;
   }
};

}

std::ostream& eddic::mtac::operator<<(std::ostream& stream, const Argument& argument){
    return visit(ArgumentToString(stream), argument);    
}
