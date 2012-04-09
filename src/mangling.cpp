//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <sstream>
#include <cctype>

#include "mangling.hpp"
#include "Utils.hpp"

#include "VisitorUtils.hpp"
#include "ast/GetTypeVisitor.hpp"

using namespace eddic;

std::string eddic::mangle(Type type){
    switch(type.base()){
        case BaseType::INT:
            return type.isArray() ? "AI" : "I";
        case BaseType::STRING:
            return type.isArray() ? "AS" : "S";
        case BaseType::BOOL:
            return type.isArray() ? "AB" : "B";
        case BaseType::FLOAT:
            return type.isArray() ? "AF" : "F";
        case BaseType::VOID:
            return "V";
    }
}

std::string eddic::mangle(const std::string& functionName, const std::vector<ParameterType>& types){
    if(functionName == "main"){
        return functionName;
    }

    std::ostringstream ss;

    ss << "_F";
    ss << functionName.length();
    ss << functionName;

    for(const ParameterType& t : types){
        ss << mangle(t.paramType);
    }

    return ss.str();
}

std::string eddic::mangle(const std::string& functionName, const std::vector<ast::Value>& values, SymbolTable& symbols){
    if(functionName == "main"){
        return functionName;
    }

    std::ostringstream ss;

    ss << "_F";
    ss << functionName.length();
    ss << functionName;

    ast::GetTypeVisitor visitor(symbols);

    for(auto& value : values){
        Type type = visit(visitor, value);
        ss << mangle(type);
    }

    return ss.str();
}

std::string eddic::unmangle(std::string mangled){
    if(mangled == "main"){
        return "main";
    }

    std::ostringstream length;
    int digits = 0;

    for(unsigned int i = 2; i < mangled.length(); ++i){
        if(isdigit(mangled[i])){
            length << mangled[i];
            ++digits;
        } else {
            break;
        }
    }

    int l = toNumber<unsigned int>(length.str());

    std::ostringstream function;
    
    for(int i = 0; i < l; ++i){
        function << mangled[i + 2 + digits];
    }

    function << '(';

    for(unsigned int i = 2 + l + digits; i < mangled.length(); ++i){
        char current = mangled[i];

        bool array = false;
        if(current == 'A'){
            array = true;
            current = mangled[++i];
        }

        if(current == 'I'){
            function << "int";
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

        if(i < mangled.length() - 1){
            function << ", ";
        }
    }

    function << ')';

    return function.str();
}
