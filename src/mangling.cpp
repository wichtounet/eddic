//=======================================================================
// Copyright Baptiste Wicht 2011-2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <sstream>
#include <cctype>

#include "mangling.hpp"
#include "assert.hpp"
#include "Utils.hpp"
#include "VisitorUtils.hpp"
#include "Type.hpp"

#include "ast/GetTypeVisitor.hpp"
#include "ast/Value.hpp"

using namespace eddic;

std::string eddic::mangle(std::shared_ptr<const Type> type){
    if(type->is_array()){
        return "A" + type->data_type()->mangle();
    }

    if(type->is_pointer()){
        return "P" + type->data_type()->mangle();
    }

    if(type->is_standard_type()){
        if(type == INT){
            return "I";
        } else if(type == CHAR){
            return "C";
        } else if(type == STRING){
            return "S";
        } else if(type == BOOL){
            return "B";
        } else if(type == FLOAT){
            return "F";
        } else if(type == VOID){
            return "V";
        }
    } 
    
    if(type->is_custom_type()){
        std::ostringstream ss;

        ss << "U";
        ss << type->type().length();
        ss << type->type();
        
        return ss.str();
    }
    
    if(type->is_template_type()){
        std::ostringstream ss;

        ss << "T";
        ss << type->type().length();
        ss << type->type();

        auto types = type->template_types();

        ss << types.size();

        for(auto& sub_type : types){
            ss << sub_type->mangle();
        }
        
        return ss.str();
    }

    eddic_unreachable("Invalid type");
}

std::string eddic::mangle(const std::string& name, const std::vector<ParameterType>& parameters, std::shared_ptr<const Type> struct_type){
    std::ostringstream ss;

    if(struct_type){
        ss << "_M";
        ss << struct_type->mangle();
    } else {
        ss << "_F";
    }

    ss << name.length();
    ss << name;

    for(auto& type : parameters){
        if(type.name() != "this"){
            ss << type.type()->mangle();
        }
    }

    return ss.str();
}

std::string eddic::mangle_ctor(const std::vector<ParameterType>& parameters, std::shared_ptr<const Type> struct_type){
    std::ostringstream ss;

    ss << "_C";
    ss << struct_type->mangle();

    for(auto& type : parameters){
        if(type.name() != "this"){
            ss << type.type()->mangle();
        }
    }

    return ss.str();
}

std::string eddic::mangle(const std::string& functionName, const std::vector<ast::Value>& values, std::shared_ptr<const Type> struct_type){
    std::ostringstream ss;

    if(struct_type){
        ss << "_M";
        ss << struct_type->mangle();
    } else {
        ss << "_F";
    }

    ss << functionName.length();
    ss << functionName;

    ast::GetTypeVisitor visitor;
    for(auto& value : values){
        auto type = visit(visitor, value);
        ss << type->mangle();
    }

    return ss.str();
}

std::string eddic::mangle_ctor(const std::vector<ast::Value>& values, std::shared_ptr<const Type> struct_type){
    std::ostringstream ss;

    ss << "_C";
    ss << struct_type->mangle();

    ast::GetTypeVisitor visitor;
    for(auto& value : values){
        auto type = visit(visitor, value);
        ss << type->mangle();
    }

    return ss.str();
}

std::string eddic::mangle_ctor(const std::vector<std::shared_ptr<const Type>>& types, std::shared_ptr<const Type> struct_type){
    std::ostringstream ss;

    ss << "_C";
    ss << struct_type->mangle();

    for(auto& type : types){
        ss << type->mangle();
    }

    return ss.str();
}

std::string eddic::mangle_dtor(std::shared_ptr<const Type> struct_type){
    std::ostringstream ss;

    ss << "_D";
    ss << struct_type->mangle();

    return ss.str();
}

std::string eddic::mangle(const std::string& functionName, const std::vector<std::shared_ptr<const Type>>& types, std::shared_ptr<const Type> struct_type){
    std::ostringstream ss;

    if(struct_type){
        ss << "_M";
        ss << struct_type->mangle();
    } else {
        ss << "_F";
    }

    ss << functionName.length();
    ss << functionName;

    for(auto& type : types){
        ss << type->mangle();
    }

    return ss.str();
}

unsigned int read_length(const std::string& mangled, unsigned int& i){
    std::ostringstream length;
    int digits = 0;

    for(; i < mangled.length(); ++i){
        if(isdigit(mangled[i])){
            length << mangled[i];
            ++digits;
        } else {
            break;
        }
    }

    return toNumber<unsigned int>(length.str());
}

std::string get_name_from_length(const std::string& mangled, unsigned int& i){
    int l = read_length(mangled, i);

    std::ostringstream name;
    
    auto start = i;
    for(; i < start + l; ++i){
        name << mangled[i];
    }

    return name.str();
}

std::string extract_type(const std::string& mangled, unsigned int& o){
    std::stringstream ss;

    char current = mangled[o];

    bool array = false;
    if(current == 'A'){
        array = true;
        current = mangled[++o];
    }

    bool pointer = false;
    if(current == 'P'){
        pointer = true;
        current = mangled[++o];
    }   
        
    ++o;

    if(current == 'I'){
        ss << "int";
    } else if(current == 'C'){
        ss << "char";
    } else if(current == 'S'){
        ss << "str";
    } else if(current == 'B'){
        ss << "bool";
    } else if(current == 'F'){
        ss << "float";
    } else if(current == 'U'){
        ss << get_name_from_length(mangled, o);
    } else if(current == 'T'){
        ss << get_name_from_length(mangled, o);
        ss << "<";

        auto params = read_length(mangled, o);
        
        for(unsigned int i = 0; i < params; ++i){
            ss << extract_type(mangled, o);

            if(i < params - 1){
                ss << ", ";
            }
        }

        ss << ">";
    }

    if(array){
        ss << "[]";
    }

    if(pointer){
        ss << "&";
    }

    return ss.str();
}

std::string eddic::unmangle(std::string mangled){
    std::ostringstream function;

    std::string prefix;
    prefix += mangled[0];
    prefix += mangled[1];
    
    unsigned int o = 2;

    //Member function
    if(prefix == "_M"){
        auto type = extract_type(mangled, o);
        function << type << "::";
    }

    //Get the name of the function
    function << get_name_from_length(mangled, o);
    function << '(';

    //Get the parameters
    while(o < mangled.length()){
        auto type = extract_type(mangled, o);
        function << type;

        if(o < mangled.length() - 1){
            function << ", ";
        }
    }

    function << ')';

    return function.str();
}
