//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <cassert>

#include "Function.hpp"
#include "Type.hpp"

using namespace eddic;

ParameterType::ParameterType(const std::string& n, std::shared_ptr<const Type> t) : name(n), paramType(t) {}

Function::Function(std::shared_ptr<const Type> ret, const std::string& n) : returnType(ret), name(n), references(0) {}

std::shared_ptr<const Type> Function::getParameterType(const std::string& name){
    for(auto& p : parameters){
        if(p.name == name){
            return p.paramType;
        }
    }
    
    assert(false && "This parameter does not exists in the given function");
}

unsigned int Function::getParameterPositionByType(const std::string& name){
    unsigned int position = 0;

    auto type = getParameterType(name);
    
    for(auto& p : parameters){
        if(p.paramType == type){
            ++position; 
        }

        if(p.name == name){
            return position;
        }
    }

    assert(false && "The parameter does not exists in the function");
}
