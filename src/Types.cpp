//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>

#include "Types.hpp"
#include "CompilerException.hpp"

using namespace eddic;

const int typeSizes[(int) Type::COUNT] = { 8, 4, 0 };

int eddic::size(Type type){
    return typeSizes[(int) type];
}

bool eddic::isType(const std::string& type){
    return type == "int" || type == "void" || type == "string";
}

Type eddic::stringToType(const std::string& type){
    if (type == "int") {
        return Type::INT;
    } else if (type == "string"){
        return Type::STRING;
    } else if(type == "void") {
        return Type::VOID;
    }

    //TODO throw CompilerException("Invalid type");
}
