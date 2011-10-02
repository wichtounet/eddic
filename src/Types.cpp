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

int eddic::size(Type type){
    return type == Type::INT ? 4 : 8;
}

bool eddic::isType(std::string type){
    return type == "int" || type == "void" || type == "string";
}

Type eddic::stringToType(std::string type){
    if (type == "int") {
        return Type::INT;
    } else if (type == "string"){
        return Type::STRING;
    } else if(type == "void") {
        return Type::VOID;
    }

    throw CompilerException("Invalid type");
}
