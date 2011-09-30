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
    return type == INT ? 4 : 8;
}

Type eddic::stringToType(std::string type){
    if (type == "int") {
        return  INT;
    } else if (type == "string"){
        return STRING;
    } else if(type == "void") {
        return VOID;
    }

    throw CompilerException("Invalid type");
}
