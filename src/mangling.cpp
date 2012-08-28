//=======================================================================
// Copyright Baptiste Wicht 2011.
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

using namespace eddic;

std::string eddic::mangle(std::shared_ptr<const Type> type){
    if(type->is_array()){
        return "A" + mangle(type->data_type());
    }

    if(type->is_pointer()){
        return "P" + mangle(type->data_type());
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

        ASSERT_PATH_NOT_TAKEN("Not a standard type");
    } else {
        std::ostringstream ss;

        ss << "C";
        ss << type->type().length();
        ss << type->type();
        
        return ss.str();
    }
}

std::string eddic::mangle(std::shared_ptr<Function> function){
    std::ostringstream ss;

    ss << "_F";

    if(!function->struct_.empty()){
        ss << function->struct_.length();
        ss << function->struct_;
    }

    ss << function->name.length();
    ss << function->name;

    for(auto type : function->parameters){
        if(type.name != "this"){
            ss << mangle(type.paramType);
        }
    }

    return ss.str();
}

std::string eddic::mangle_ctor(const std::shared_ptr<Function> function){
    std::ostringstream ss;

    ss << "_C";

    ss << function->struct_.length();
    ss << function->struct_;

    for(auto type : function->parameters){
        if(type.name != "this"){
            ss << mangle(type.paramType);
        }
    }

    return ss.str();
}

std::string eddic::mangle_dtor(const std::shared_ptr<Function> function){
    std::ostringstream ss;

    ss << "_D";

    ss << function->struct_.length();
    ss << function->struct_;

    return ss.str();
}

std::string eddic::mangle(const std::string& functionName, const std::vector<ast::Value>& values, const std::string& struct_){
    std::ostringstream ss;

    ss << "_F";

    if(!struct_.empty()){
        ss << struct_.length();
        ss << struct_;
    }

    ss << functionName.length();
    ss << functionName;

    ast::GetTypeVisitor visitor;
    for(auto& value : values){
        auto type = visit(visitor, value);
        ss << mangle(type);
    }

    return ss.str();
}

std::string eddic::mangle_ctor(const std::vector<ast::Value>& values, const std::string& struct_){
    std::ostringstream ss;

    ss << "_C";

    ss << struct_.length();
    ss << struct_;

    ast::GetTypeVisitor visitor;
    for(auto& value : values){
        auto type = visit(visitor, value);
        ss << mangle(type);
    }

    return ss.str();
}

std::string eddic::mangle_dtor(const std::string& struct_){
    std::ostringstream ss;

    ss << "_D";

    ss << struct_.length();
    ss << struct_;

    return ss.str();
}

std::string eddic::mangle(const std::string& functionName, const std::vector<std::shared_ptr<const Type>>& types, const std::string& struct_){
    std::ostringstream ss;

    ss << "_F";

    if(!struct_.empty()){
        ss << struct_.length();
        ss << struct_;
    }

    ss << functionName.length();
    ss << functionName;

    for(auto type : types){
        ss << mangle(type);
    }

    return ss.str();
}

std::string get_name_from_length(const std::string& mangled, unsigned int& i){
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

    int l = toNumber<unsigned int>(length.str());

    std::ostringstream name;
    
    auto start = i;
    for(; i < start + l; ++i){
        name << mangled[i];
    }

    return name.str();
}

std::string eddic::unmangle(std::string mangled){
    unsigned int o = 2;

    std::ostringstream function;

    function << get_name_from_length(mangled, o);

    //Test if inside a struct
    if(isdigit(mangled[o])){
        function << "::";
        
        function << get_name_from_length(mangled, o);
    }

    function << '(';

    for(; o < mangled.length(); ++o){
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

        if(current == 'I'){
            function << "int";
        } else if(current == 'C'){
            function << "char";
        } else if(current == 'S'){
            function << "string";
        } else if(current == 'B'){
            function << "bool";
        } else if(current == 'F'){
            function << "float";
        } 

        if(array){
            function << "[]";
        }

        if(pointer){
            function << "&";
        }

        if(o < mangled.length() - 1){
            function << ", ";
        }
    }

    function << ')';

    return function.str();
}
